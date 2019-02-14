//
// Created by 赵立伟 on 2018/11/29.
//

#include <fcntl.h>
#include <list>
#include "AOFHandler.h"
#include "../io/FileFio.h"
#include "../dataStructure/skiplist/SkipList.cpp"
#include "../dataStructure/dict/Dict.cpp"

bool AOFHandler::stopSendingDiff = false;

AOFHandler::AOFHandler() {

}

int AOFHandler::start() {
    this->lastFsync = time(NULL);
    this->fd = open(this->fileName, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (-1 == this->fd) {
        coordinator->getLogHandler()->logWarning(
                "FLYDB needs to enable the AOF but can't open the "
                "append only file %s : %s",
                this->fileName,
                strerror(errno));
        return -1;
    }

    /**
     * 判断是否有fdb子进程在执行:
     *      如果是，则稍候调度执行aof
     *      否则，直接进行aof rewrite操作
     **/
    if (coordinator->getFdbHandler()->haveChildPid()) {
        this->scheduled = true;
        coordinator->getLogHandler()->logWarning(
                "AOF was enabled but there is already "
                "a child process saving an FDB file on disk."
                "An AOF background was scheduled to start when possible.");
    } else if (-1 == rewriteBackground()) {
        close(this->fd);
        coordinator->getLogHandler()->logWarning(
                "Redis needs to enable the AOF but can't trigger "
                "a background AOF rewrite operation. Check the above "
                "logs for more info about the error.");
        return -1;
    }

    /**
     * wait for the rewrite complete:
     * 无论是schedule开始运行还是直接运行完，都是等待rewrite完毕，
     * 以便于之后执行append data on disk
     **/
    this->state = AOF_WAIT_REWRITE;
    return 1;
}

int AOFHandler::rewriteBackground() {
    /** set stop sending diff flag to false */
    stopSendingDiff = false;

    /**
     * 有aof或者fdb子进程在运行，则直接返回
     **/
    if (coordinator->getFdbHandler()->haveChildPid()
        || this->haveChildPid()) {
        return -1;
    }

    /** 打开主线程与子进程之间的管道 */
    if (-1 == coordinator->openAllPipe()) {
        return -1;
    }
    if (-1 == coordinator->getAofDataPipe()->setReadNonBlock()
        || -1 == coordinator->getAofDataPipe()->setWriteNonBlock()
        || -1 == coordinator->getChildInfoPipe()->setReadNonBlock()) {
        coordinator->closeAllPipe();
        return -1;
    }

    /** 创建文件事件: 用于parent处理child发送过来的ack */
    if (-1 == this->coordinator->getEventLoop()->createFileEvent(
            coordinator->getAofAckToParentPipe()->getReadPipe(),
            ES_READABLE,
            childPipeReadable,
            NULL)) {
        coordinator->closeAllPipe();
        return -1;
    }

    AbstractFlyServer *flyServer = coordinator->getFlyServer();
    pid_t childPid;
    if (0 == (childPid = fork())) {
        /**
         * child thread
         **/
        flyServer->closeListeningSockets(false);
        if (1 == this->rewriteAppendOnlyFile()) {
            /** send child info */
            coordinator->getChildInfoPipe()->sendInfo(PIPE_TYPE_AOF, 0);
            exit(0);
        }

        /** write failure */
        exit(1);
    } else {
        /** parent */
        if (-1 == childPid) {    /** 创建子线程失败 */
            coordinator->closeAllPipe();
        }

        this->childPid = childPid;
        this->scheduled = false;
        flyServer->updateDictResizePolicy();
    }

    return 1;
}

void AOFHandler::backgroundSaveDone(int exitCode, int bySignal) {
    // todo: complete these code
    if (0 == bySignal && 0 == exitCode) {
        this->coordinator->getLogHandler()->logNotice(
                "Background saving terminated with success");
    } else if (0 == bySignal && 0 != exitCode) {
        this->coordinator->getLogHandler()->logWarning(
                "Background AOF rewrite terminated with error");
    } else {
        this->coordinator->getLogHandler()->logWarning(
                "Background saving terminated by signal %d", bySignal);
    }
}

int AOFHandler::rewriteAppendOnlyFile() {
    char tmpfile[256];

    /** 获取临时aof文件 */
    snprintf(tmpfile, sizeof(tmpfile), "temp-rewriteaof-%d.aof", getpid());
    FILE *fp = fopen(tmpfile, "w");
    if (NULL == fp) {
        this->coordinator->getLogHandler()->logWarning(
                "Opening the temp file for AOF rewrite "
                "in rewriteAppendOnlyFile(): %s",
                strerror(errno));
        return -1;
    }

    /** 将当前状态下的数据持久化 */
    off_t autosync = this->rewriteIncrementalFsync ? AOF_AUTOSYNC_BYTES : 0;
    FileFio* fio = FileFio::Builder().file(fp).autosync(autosync).build();
    /** 如果使用混合持久化，则调用fdb进行快照持久化 */
    if (this->useFdbPreamble) {
        FDBSaveInfo *fdbSaveInfo = new FDBSaveInfo();
        if (-1 == coordinator->getFdbHandler()->saveToFio(
                fio, RDB_SAVE_AOF_PREAMBLE, fdbSaveInfo)) {
            fclose(fp);
            unlink(tmpfile);
            return -1;
        }
    } else {
        if (-1 == this->rewriteAppendOnlyFileFio(fio)) {
            fclose(fp);
            unlink(tmpfile);
            return -1;
        }
    }

    /** 刷新至磁盘中 */
    if (EOF == fflush(fp) || -1 == fsync(fileno(fp))) {
        coordinator->getLogHandler()->logWarning(
                "Write error saving DB on disk: %s", strerror(errno));
        fclose(fp);
        unlink(tmpfile);
        return -1;
    }

    /** write aof diff data which is from parent to aof pipe */
    return this->rewriteAppendOnlyFileDiff(tmpfile, fio);
}

int AOFHandler::rewriteAppendOnlyFileDiff(char *tmpfile, FileFio *fio) {
    FILE *fp = fio->getFp();
    /**
     * 从parent读取持久化之后的aof数据
     *      只要一直有从parent到本线程的数据，那尽量多读一段时间（最多不超过1000ms）
     *      否则，如果持续读20次都没有数据，那么退出循环
     **/
    uint64_t start = miscTool->mstime();
    int loop = 0;
    while (loop < 20 && miscTool->mstime() - start < 1000) {
        if (coordinator->getNetHandler()->wait(
                coordinator->getAofDataPipe()->getReadPipe(),
                ES_READABLE,
                1) <= 0) {
            loop++;
            continue;
        }

        loop = 0;
        this->readDiffFromParent();
    }

    /** 向parent发送ack，用以通知parent停止发送aof diff数据 */
    if (1 != write(
            coordinator->getAofAckToParentPipe()->getWritePipe(), "!", 1)) {
        coordinator->getLogHandler()->logWarning(
                "Write error saving DB on disk: %s", strerror(errno));
        fclose(fp);
        unlink(tmpfile);
        return -1;
    }

    /** 设置从parent读取ack的管道为非阻塞, 防止下面的syncRead block住 */
    int ackToChildReadFd = coordinator->getAofAckToChildPipe()->getReadPipe();
    if (-1 == coordinator->getNetHandler()->setBlock(
            NULL, ackToChildReadFd, 0)) {
        coordinator->getLogHandler()->logWarning(
                "Write error saving DB on disk: %s", strerror(errno));
        fclose(fp);
        unlink(tmpfile);
        return -1;
    }

    /** 读取parent发送来的ack */
    char byte;
    if (1 != coordinator->getNetHandler()->syncRead(
            ackToChildReadFd, &byte, 1, 5000) || byte != '!') {
        coordinator->getLogHandler()->logWarning(
                "Write error saving DB on disk: %s", strerror(errno));
        fclose(fp);
        unlink(tmpfile);
        return -1;
    }
    coordinator->getLogHandler()->logNotice(
            "Parent agreed to stop sending diffs. Finalizing AOF...");

    /**
     * 最后尝试读取一次parent to child info pipe,
     * 防止在互相ack过程中有数据发送出来
     **/
    this->readDiffFromParent();

    /** 将childDiff中的数据写入文件 */
    coordinator->getLogHandler()->logNotice(
            "Concatenating %.2f MB of AOF diff received from parent.");
    if (0 == fio->write(this->childDiff.c_str(), this->childDiff.length())) {
        coordinator->getLogHandler()->logWarning(
                "Write error saving DB on disk: %s", strerror(errno));
        fclose(fp);
        unlink(tmpfile);
        return -1;
    }

    /** 刷新至磁盘中 */
    if (EOF == fflush(fp) || -1 == fsync(fileno(fp)) || -1 == fclose(fp)) {
        coordinator->getLogHandler()->logWarning(
                "Write error saving DB on disk: %s", strerror(errno));
        fclose(fp);
        unlink(tmpfile);
        return -1;
    }

    /** rename file to real file name */
    if (-1 == rename(tmpfile, this->fileName)) {
        this->coordinator->getLogHandler()->logWarning(
                "Error moving temp append only file "
                "on the final destination: %s", strerror(errno));
        unlink(tmpfile);
        return -1;
    }
    this->coordinator->getLogHandler()->logNotice(
            "SYNC append only file rewrite performed");
    return 1;
}

int AOFHandler::readDiffFromParent() {
    char buf[65536];
    int totalRead = 0, onceRead;
    while ((onceRead = read(coordinator->getAofDataPipe()->getReadPipe(),
            buf, sizeof(buf))) > 0) {
        this->childDiff += buf;
        totalRead += onceRead;
    }

    return totalRead;
}

int AOFHandler::dbScan(void *priv,
                        std::string key,
                        std::shared_ptr<FlyObj> val) {
    FioAndflyDB *fioAndflyDB = reinterpret_cast<FioAndflyDB *>(priv);
    AbstractFlyDB *flyDB = fioAndflyDB->flyDB;

    /** 存入该key-value */
    int64_t expire = flyDB->getExpire(key);
    return flyDB->getCoordinator()
            ->getAofHandler()
            ->saveKeyValuePair(fioAndflyDB->fio, key, val, expire);

}

int AOFHandler::saveKeyValuePair(Fio *fio,
                                 std::string key,
                                 std::shared_ptr<FlyObj> val,
                                 int64_t expireTime) {
    /** 如果该key已经过期，直接返回 */
    if (-1 != expireTime && expireTime < miscTool->mstime()) {
        return -1;
    }

    if (FLY_TYPE_STRING == val->getType()) {
        std::string cmd = "*3\r\n$3\r\nSET\r\n";
        if (0 == fio->writeBulkString(cmd)
            || 0 == fio->writeBulkString(key)
            || 0 == fio->writeBulkString(*(std::string*)val->getPtr())) {
            return -1;
        }
    } else if (FLY_TYPE_LIST == val->getType()) {
        this->rewriteList(fio, key, reinterpret_cast<
                std::list<std::string>*>(val->getPtr()));
    } else if (FLY_TYPE_SKIPLIST == val->getType()) {
        this->rewriteSkipList(fio, key, reinterpret_cast<
                SkipList<std::string> *>(val->getPtr()));
    } else if (FLY_TYPE_HASH == val->getType()) {
        this->rewriteHashTable(fio, key, reinterpret_cast<
                Dict<std::string, std::string>*>(val->getPtr()));
    } else if (FLY_TYPE_SET == val->getType()) {
        this->rewriteIntSet(fio, key, reinterpret_cast<IntSet*>(val->getPtr()));
    } else {
        coordinator->getLogHandler()->logWarning("Unknown object type!");
        exit(1);
    }

    /** 如果有过期时间，则写入 */
    if (-1 != expireTime) {
        std::string cmd = "*3\r\n$9\r\nPEXPIREAT\r\n";
        if (0 == fio->writeBulkString(cmd)
            || 0 == fio->writeBulkString(key)
            || 0 == fio->writeBulkInt64(expireTime)) {
            return -1;
        }
    }

    return 1;
}

int AOFHandler::rewriteAppendOnlyFileFio(Fio *fio) {
    AbstractFlyServer *flyServer = coordinator->getFlyServer();
    int dbCount = flyServer->getFlyDBCount();
    for (int i = 0; i < dbCount; i++) {
        std::string selectcmd = "2\r\n$6\r\nSELECT\r\n";
        AbstractFlyDB *flyDB = flyServer->getFlyDB(i);
        /** 写入select i */
        if (0 == fio->write(selectcmd.c_str(), selectcmd.size())) {
            return -1;
        }
        if (0 == fio->writeBulkInt64(i)) {
            return -1;
        }

        flyDB->dictScan(fio, dbScan);
    }

    return 1;
}

int AOFHandler::rewriteList(Fio *fio,
                           std::string key,
                           std::list<std::string> *val) {
    int valCount = val->size();

    /** "*{count}\r\n$5\r\nRPUSH\r\n${keylength}\r\n{key}\r\n" */
    if (0 == fio->writeBulkCount('*', valCount + 2)
        || 0 == fio->writeBulkString("rpush")
        || 0 == fio->writeBulkString(key)) {
        return -1;
    }

    /** val list */
    for (auto item : *val) {
        fio->writeBulkString(item);
    }

    return 1;
}

int AOFHandler::rewriteSkipList(Fio *fio,
                                std::string key,
                                SkipList<std::string> *skipList) {
    int length = skipList->getLength();

    /** "*{count}\r\n$8\r\sortpush\r\n${keylength}\r\n{key}\r\n" */
    if (0 == fio->writeBulkCount('*', length + 2)
        || 0 == fio->writeBulkString("sortpush")
        || 0 == fio->writeBulkString(key)) {
        return -1;
    }

    /** 存入val list */
    skipList->scanAll(skipListSaveProc,
                      new FioAndCoord(fio, this->coordinator));

    return 1;
}

void AOFHandler::skipListSaveProc(void *priv, const std::string &obj) {
    FioAndCoord *fioAndCoord = reinterpret_cast<FioAndCoord *>(priv);
    Fio *fio = fioAndCoord->fio;

    /** 存入val */
    fio->writeBulkString(obj);
}

int AOFHandler::rewriteHashTable(Fio *fio,
                                 std::string key,
                                 Dict<std::string, std::string> *dict) {
    int size = dict->size();

    /** "*{count}\r\n$5\r\hmset\r\n${keylength}\r\n{key}\r\n" */
    if (0 == fio->writeBulkCount('*', size + 2)
        || 0 == fio->writeBulkString("hmset")
        || 0 == fio->writeBulkString(key)) {
        return -1;
    }

    FioAndCoord* fioAndCoord = new FioAndCoord(fio, this->coordinator);
    uint32_t nextCur = 0;
    do {
        nextCur = dict->dictScan(
                nextCur,
                1,
                dictSaveScan,
                fioAndCoord);
    } while (nextCur != 0);

    delete fioAndCoord;
    return 1;
}

int AOFHandler::dictSaveScan(void *priv,
                              std::string key,
                              std::string val) {
    FioAndCoord *fioAndCoord = reinterpret_cast<FioAndCoord *>(priv);
    Fio *fio = fioAndCoord->fio;

    /** 写入key-value，如果出错，返回-1 */
    if (0 == fio->writeBulkString(key)
        || 0 == fio->writeBulkString(val)) {
        return -1;
    }

    return 1;
}

/** 接收child发送的通知，该通知用于告知parent不要继续发送aof diff数据了 */
void AOFHandler::childPipeReadable(const AbstractCoordinator *coordinator,
                                   int fd,
                                   void *clientdata,
                                   int mask) {
    char byte;
    if (1 == read(fd, &byte, 1) && '!' == byte) {
        coordinator->getLogHandler()->logWarning(
                "AOF rewrite child asks to stop sending diffs.");
        int writefd = coordinator->getAofAckToChildPipe()->getWritePipe();
        stopSendingDiff = true;
        if (1 != write(writefd, "!", 1)) {
            coordinator->getLogHandler()->logWarning(
                    "Can`t send ACK to AOF child: %s",
                    strerror(errno));
        }

    }

    /** 停止parent监控消息*/
    coordinator->getEventLoop()->deleteFileEvent(
            coordinator->getAofAckToParentPipe()->getReadPipe(),
            ES_READABLE);
}

int AOFHandler::rewriteIntSet(Fio *fio,
                              std::string key,
                              IntSet *intset) {
    int32_t length = intset->lenth();

    /** "*{count}\r\n$5\r\nset\r\n${keylength}\r\n{key}\r\n" */
    if (0 == fio->writeBulkCount('*', length + 2)
        || 0 == fio->writeBulkString("set")
        || 0 == fio->writeBulkString(key)) {
        return -1;
    }

    for (int i = 0; i < length; i++) {
        int64_t val = 0;
        intset->get(i, &val);
        if (0 == fio->writeBulkInt64(val)) {
            return -1;
        }
    }

    return 1;
}

void AOFHandler::removeTempFile(pid_t childpid) {
    char temp[1024];
    snprintf(temp, 256, "temp-rewriteaof-bg-%d.aof", (int) childpid);
    unlink(temp);
}

pid_t AOFHandler::getChildPid() const {
    return this->childPid;
}

void AOFHandler::setChildPid(pid_t childPid) {
    this->childPid = childPid;
}

bool AOFHandler::haveChildPid() const {
    return -1 != this->childPid;
}

bool AOFHandler::IsStateOn() const {
    return this->state == AOF_ON;
}

void AOFHandler::setState(AOFState aofState) {
    this->state = aofState;
}

void AOFHandler::setCoordinator(AbstractCoordinator *coordinator) {
    this->coordinator = coordinator;
}

void AOFHandler::setFileName(char *fileName) {
    this->fileName = fileName;
}

void AOFHandler::setUseFdbPreamble(bool useFdbPreamble) {
    this->useFdbPreamble = useFdbPreamble;
}

void AOFHandler::setFsyncStragy(int stragy) {
    this->fsyncStragy = stragy;
}

void AOFHandler::setRewriteIncrementalFsync(bool rewriteIncrementalFsync) {
    this->rewriteIncrementalFsync = rewriteIncrementalFsync;
}


bool AOFHandler::isScheduled() const {
    return this->scheduled;
}

void AOFHandler::setScheduled(bool scheduled) {
    this->scheduled = scheduled;
}

