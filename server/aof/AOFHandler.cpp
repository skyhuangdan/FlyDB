//
// Created by 赵立伟 on 2018/11/29.
//

#include <fcntl.h>
#include <list>
#include <cassert>
#include <signal.h>
#include <sys/stat.h>
#include "AOFHandler.h"
#include "../io/FileFio.h"
#include "../dataStructure/skiplist/SkipList.cpp"
#include "../dataStructure/dict/Dict.cpp"
#include "../bio/BIODef.h"
#include "../net/NetHandler.h"

bool AOFHandler::stopSendingDiff = false;

AOFHandler::AOFHandler() {

}

/** 打开AOF, 首先rewrite一遍AOF数据，并在保存完之后设置AOF状态为AOF_ON */
int AOFHandler::start() {
    this->lastFsync = time(NULL);
    this->fd = open(this->fileName, O_WRONLY | O_APPEND | O_CREAT, 0644);
    assert(AOF_OFF == this->state);
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

    this->state = AOF_WAIT_REWRITE;
    return 1;
}

/**
 * 写入并同步:
 *  1.由于要求写入aof在给客户回复之前，客户端的回复在serverCron中,
 *    所以将调用flushd的操作放在beforeSleep中
 *  2.对于everysec模式，采用background fsync（bio中执行fsync）
 *    如果bio中已经有background fsync操作，则不执行write操作，
 *    因为fsync操作会阻塞write(除非设置了force)
 **/
void AOFHandler::flush(bool force) {
    if (0 == this->buf.length()) {
        return;
    }

    AbstractFlyServer *flyServer = this->coordinator->getFlyServer();
    bool syncInProgress = false;
    if (AOF_FSYNC_EVERYSEC == this->fsyncStragy) {
        syncInProgress = 0 < this->coordinator->getBioHandler()
                ->getPendingJobCount(BIO_AOF_FSYNC);
    }

    /**
     * 如果是每秒同步一次，并且非强制同步
     **/
    if (AOF_FSYNC_EVERYSEC == this->fsyncStragy && !force) {
        /**
         * 如果当前有fsync操作，则推迟当前操作，最大推迟2s
         */
        if (syncInProgress) {
            /** 刚开始进入推迟，记录最初推迟时间，便于计算总统的推迟2s时间 */
            if (0 != this->flushPostponedStart) {
                this->flushPostponedStart = flyServer->getNowt();
                return;
            } else if (flyServer->getNowt() - this->flushPostponedStart < 2) {
                return;
            }

            /** 如果推迟超过了2s，则进行fsync，不继续推迟了 */
            this->delayedFsync++;
            coordinator->getLogHandler()->logNotice(
                    "Asynchronous AOF fsync is taking too long(disk is busy?). "
                    "Writing the AOF buffer without waiting for fsync to "
                    "complete, this may slow down FLYDB.");
        }
    }
    /** reset to zero */
    this->flushPostponedStart = 0;

    /** 执行写入操作 */
    this->doRealWrite();

    /** 执行同步操作 */
    this->doRealFsync(syncInProgress);
}

int AOFHandler::stop() {
    assert(AOF_OFF != this->state);

    /** flush aof file and close it */
    this->flush(true);
    aof_fsync(this->fd);
    close(this->fd);

    this->fd = -1;
    this->state = AOF_OFF;
    if (-1 != this->childPid) {
        int statloc;
        /** 如果杀不死，则一直等其(子进程)执行完 */
        if (kill(this->childPid, SIGUSR1) == -1) {
            while (this->childPid != wait3(&statloc, 0, NULL)) {
                continue;
            }
        }

        /** 删除aof临时文件 */
        this->removeTempFile();
        this->childPid = -1;
        this->rewriteTimeStart = -1;
        /** 清空aof rewrite buffer */
        this->clearRewriteBuffer();
        /** 清空文件事件 */
        this->clearFileEvent();
        /** 关闭所有管道 */
        this->coordinator->closeAllPipe();
    }

    return 1;
}

/** 通过创建伪客户端来加载aof文件 */
int AOFHandler::load() {
    FILE *fp = NULL;
    if (NULL == (fp = fopen(this->fileName, "r"))) {
        coordinator->getLogHandler()->logWarning(
                "Fatal Error: cann`t open the aof file: %s", strerror(errno));
        exit(1);
    }

    struct stat sb;
    /** 当前aof文件大小是0，表明该db从来没有进行过写操作 */
    if (-1 != fstat(fileno(fp), &sb) && 0 == sb.st_size) {
        this->currentSize = 0;
        fclose(fp);
        return -1;
    }

    AbstractFlyServer *flyServer = coordinator->getFlyServer();
    /** 标记正在加载持久化文件 */
    flyServer->startToLoad();

    char sig[5];
    if (5 == fread(sig, 1, sizeof(sig), fp)
        && 0 == memcmp(sig, "FLYDB", 5)
        && 0 == fseek(fp, 0, SEEK_SET)) {
        coordinator->getLogHandler()->logWarning(
                "Reading FDB from AOF file...");

        if (coordinator->getFdbHandler()->loadFromFile(fp, NULL) <= 0) {
            coordinator->getLogHandler()->logWarning(
                    "Error reading FDB preamble of the AOF file!");
            fclose(fp);
            flyServer->stopLoad();
        } else {
            coordinator->getLogHandler()->logWarning(
                    "Reading the remaining AOF tail...");
        }
    } else {
        fclose(fp);
        flyServer->stopLoad();
    }

    /** load剩余的AOF文件 */
    this->loadRemaindingAOF(fp);

    /** 标记停止load */
    flyServer->stopLoad();
}

int AOFHandler::loadRemaindingAOF(FILE *fp) {
    AbstractFlyServer *flyServer = coordinator->getFlyServer();

    /** create fake client: fake client`s id is -1 */
    AbstractFlyClient *fakeClient = coordinator->getFlyClientFactory()
            ->getFlyClient(-1, coordinator, flyServer->getFlyDB(0));

    /** 从AOF文件中读取命令数据 */
    while (1) {
        char buf[128];
        int readCnt = read(fileno(fp), buf, sizeof(buf));
        if (-1 == readCnt) {            /** 读取失败 */
            delete fakeClient;
            return -1;
        } else if (0 == readCnt) {      /** 文件已经读取完毕 */
            break;
        }
        fakeClient->addToQueryBuf(buf);

        /** 解析query buf, 并执行解析出的command */
        coordinator->getNetHandler()
                ->processInputBuffer(coordinator, fakeClient);

    }

    delete fakeClient;
    return 1;
}

/** 具体的AOF rewrite操作都是放在子进程里的，这里与FDB不同 */
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

    /** 打开主进程与子进程之间的管道 */
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
         * child process
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
        if (-1 == childPid) {    /** 创建子进程失败 */
            coordinator->closeAllPipe();
        }

        this->childPid = childPid;
        this->scheduled = false;
        this->rewriteTimeStart = flyServer->getNowt();
        this->state = AOF_WAIT_REWRITE;
        flyServer->updateDictResizePolicy();
    }

    return 1;
}

void AOFHandler::backgroundSaveDone(int exitCode, int bySignal) {
    if (0 != bySignal && 0 == exitCode) {
        this->coordinator->getLogHandler()->logNotice(
                "Background saving terminated with success");
        this->terminateWithSuccess();
    } else if (0 != bySignal && 0 != exitCode) {
        this->coordinator->getLogHandler()->logWarning(
                "Background AOF rewrite terminated with error");
    } else {
        this->coordinator->getLogHandler()->logWarning(
                "Background saving terminated by signal %d", bySignal);
    }

    /** do some clean up operation */
    this->rewriteCleanup();

    return;
}

void AOFHandler::rewriteCleanup() {
    coordinator->closeAllPipe();
    this->removeTempFile();
    this->clearRewriteBuffer();
    this->childPid = -1;
    this->rewriteTimeLast = coordinator->getFlyServer()->getNowt()
            - this->rewriteTimeStart;
    this->rewriteTimeStart = -1;
    /**
     * 如果运行失败, 设置schedule，使在servercron中重新运行rewrite
     * 如果正常rewrite完，state会被设置成AOF_ON，
     **/
    if (AOF_WAIT_REWRITE == this->state) {
        this->scheduled = 1;
    }
}

/** 成功执行完rewrite操作 */
void AOFHandler::terminateWithSuccess() {
    char tempfile[256];
    snprintf(tempfile,
             sizeof(tempfile),
             "temp-rewriteaof-bg-%d.aof",
             this->childPid);
    int newfd = open(tempfile, O_WRONLY | O_APPEND);
    if (-1 == newfd) {
        coordinator->getLogHandler()->logWarning(
                "Unable to open the temporary AOF produced by the child: %s",
                strerror(errno));
        this->rewriteCleanup();
        return;
    }

    /** 将剩余的所有buffer block list中数据写入新文件中 */
    if (-1 == this->rewriteBufferWriteToFile(newfd)) {
        close(newfd);
        this->rewriteCleanup();
        return;
    }

    /**
     * 剩下我们要做的就是rename临时文件名字成配置aof文件名，以便用于之后的aof写入。
     * 在删除旧文件时，我们不想让close和rename阻塞服务器，这里有两种场景下可以导致阻塞发生：
     *      场景1: rename函数导致的。AOF被禁止了（即aof文件关闭了），但是当前有一次aof rewrite，
     *            临时文件将会被重命名成配置的aof文件名。当该文件已经存在时，它将会被unlink，该操作会block服务器
     *      场景2. close函数导致的。AOF是enable的（此时aof文件是打开的），rewrite AOF将会立即开始接收写入。
     *            在临时文件被rename成配置文件名之后（rename不会导致该文件被unlink），原来的AOF文件描述符将会被close，
     *            因为该文件描述符是对该文件的最后一个引用了，所以关闭该文件描述符将会导致这个文件被unlink，这样也会block服务器。
     * 为了消除block的影响，我们使用background thread来处理：
     *      1.首先，我们将场景1转换成场景2。当原有aof文件存在时，这时使用open功能，将原有aof文件打开
     *      2.这样在rename操作之后的close操作会导致unlink，将该操作放入background thread来处理，我们就不必关心其关闭操作持续多久了
     */
    int oldfd;
    if (this->IsStateOff()) {
        /** aof disable */
        oldfd = open(this->fileName, O_WRONLY | O_APPEND);
    } else {
        /** aof enable:
         * 令oldfd为-1，用于在rename失败的时候判断是否手动打开过aof原文件
         **/
        oldfd = -1;
    }

    /** rename文件 */
    if (-1 == rename(tempfile, this->fileName)) {
        close(newfd);
        if (-1 != oldfd) {
            close(oldfd);
        }
        this->rewriteCleanup();
        return;
    }

    if (this->IsStateOff()) {
        /** 如果aof是关闭状态，没必要打开aof文件 */
        close(newfd);
    } else {    /** aof是打开状态 */
        this->fd = newfd;
        /** do fsync operation */
        if (AOF_FSYNC_ALWAYS == this->fsyncStragy) {
            aof_fsync(this->fd);
        } else if (AOF_FSYNC_EVERYSEC == this->fsyncStragy) {
            /** every second模式都是background fsync */
            coordinator->getBioHandler()->createBackgroundJob(
                    BIO_AOF_FSYNC,
                    reinterpret_cast<void*>(this->fd),
                    NULL,
                    NULL);
        }
        this->updateCurrentSize();
        this->rewriteBaseSize = this->currentSize;
        this->buf.clear();
        /** rewrite完毕，设置该AOF state为打开 */
        if (AOF_WAIT_REWRITE == this->state) {
            this->state = AOF_ON;
        }

        /** 真正导致unlink阻塞的关闭文件操作交给background thread */
        if (-1 != oldfd) {
            coordinator->getBioHandler()->createBackgroundJob(
                    BIO_CLOSE_FILE,
                    reinterpret_cast<void*>(oldfd),
                    NULL,
                    NULL);
        }
    }

}

void AOFHandler::updateCurrentSize() {
    struct stat sb;
    if (-1 == fstat(this->fd, &sb)) {
        coordinator->getLogHandler()->logWarning(
                "Unable to obtain the AOF file length. stat: %s",
                strerror(errno));
    }

    this->currentSize = sb.st_size;
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

/**
 * 保存子进程在进行aof持久化过程中主进程执行的命令列表（主进程发送给该子进程），
 * 用于减少执行完aof后需要主进程进行保存的工作量，即在backgroundSaveDone函数中进行保存
 * buffer block list的工作量，减少主进程进行持久化所造成的阻塞
 * */
int AOFHandler::rewriteAppendOnlyFileDiff(char *tmpfile, FileFio *fio) {
    FILE *fp = fio->getFp();
    /**
     * 从parent读取持久化之后的aof数据
     *      只要一直有从parent到本进程的数据，那尽量多读一段时间（最多不超过1000ms）
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
    if (1 != write(coordinator->getAofAckToParentPipe()->getWritePipe(),
                   "!", 1)) {
        coordinator->getLogHandler()->logWarning(
                "Sending ack to parent error: %s", strerror(errno));
        fclose(fp);
        unlink(tmpfile);
        return -1;
    }

    /** 设置从parent读取ack的管道为非阻塞, 防止下面的syncRead block住 */
    int ackToChildReadFd = coordinator->getAofAckToChildPipe()->getReadPipe();
    if (-1 == coordinator->getNetHandler()->setBlock(
            NULL, ackToChildReadFd, 0)) {
        coordinator->getLogHandler()->logWarning(
                "Can`t set nonblock: %s", strerror(errno));
        fclose(fp);
        unlink(tmpfile);
        return -1;
    }

    /** 读取parent发送来的ack */
    char byte;
    if (1 != coordinator->getNetHandler()->syncRead(
            ackToChildReadFd, &byte, 1, 5000) || byte != '!') {
        coordinator->getLogHandler()->logWarning(
                "Cann`t get \"!\" from parent : %s", strerror(errno));
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
    /** 当接parent收到'!'时，代表child通知parent停止传输 */
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

        /** 停止接收parent发来的消息*/
        coordinator->getEventLoop()->deleteFileEvent(
                coordinator->getAofAckToParentPipe()->getReadPipe(),
                ES_READABLE);
    }
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

void AOFHandler::rewriteBufferAppend(unsigned char *s, uint64_t len) {
    RewriteBufBlock *block = this->rewriteBufBlocks.back();
    while (len > 0) {
        /** 如果block不为空，则先向该block中填入*/
        if (NULL != block) {
            int writeBytes = block->free < len ? block->free : len;
            memcpy(block->buf + block->used, s, writeBytes);
            block->free -= writeBytes;
            block->used += writeBytes;
            len -= writeBytes;
        }
        /** 如果该block能够完全容下，直接返回 */
        if (len <= 0) {
            return;
        }

        block = new RewriteBufBlock();
        this->rewriteBufBlocks.push_back(block);
    }

    /** 添加向child process进行同步的文件事件 */
    int aofDataWriteFd = coordinator->getAofDataPipe()->getWritePipe();
    AbstractEventLoop *eventLoop = coordinator->getEventLoop();
    if (eventLoop->getFileEvents(aofDataWriteFd) <= 0) {
        eventLoop->createFileEvent(aofDataWriteFd,
                                   ES_WRITABLE,
                                   childWriteDiffData,
                                   NULL);
    }

    return;
}

void AOFHandler::childWriteDiffData(const AbstractCoordinator *coorinator,
                                    int fd,
                                    void *clientdata,
                                    int mask) {
    AbstractAOFHandler *aofHandler = coorinator->getAofHandler();
    AbstractEventLoop *eventLoop = coorinator->getEventLoop();

    /** 循环一直到所有buffer都写完 */
    uint64_t totalWrittenOneBlock = 0;
    while(1) {
        RewriteBufBlock* block = aofHandler->getFrontRewriteBufBlock();

        /**
         * 判断是否需要删除相应的文件事件：
         *  1.如果block为空，说明已经全部写完
         *  2.如果标记了停止向child process发送
         * 满足1或者2两者中的一个，则删除该文件事件
         **/
        if (AOFHandler::stopSendingDiff || NULL == block) {
            eventLoop->deleteFileEvent(
                    coorinator->getAofDataPipe()->getWritePipe(),
                    ES_WRITABLE);
            return;
        }

        /** 向该管道中写入:
         *      此处与redis相比，减少了memmove的可能性，进行了优化
         * */
        int written = write(coorinator->getAofDataPipe()->getWritePipe(),
                            block->buf, block->used);
        if (written <= 0) { /** 写入出错 */
            block->used -= totalWrittenOneBlock;
            block->free += totalWrittenOneBlock;
            memmove(block->buf,
                    block->buf + totalWrittenOneBlock,
                    block->used - totalWrittenOneBlock);
            return;
        }

        totalWrittenOneBlock += written;
        /** 如果该block已经写完，则将其删除 */
        if (totalWrittenOneBlock == block->used) {
            aofHandler->popFrontRewriteBufBlock();
            totalWrittenOneBlock = 0;
        }
    }
}

RewriteBufBlock* AOFHandler::getFrontRewriteBufBlock() const {
    return this->rewriteBufBlocks.front();
}

void AOFHandler::popFrontRewriteBufBlock() {
    /** 先释放空间，再从列表中摘除掉 */
    RewriteBufBlock *block = this->rewriteBufBlocks.front();
    delete block;
    this->rewriteBufBlocks.pop_front();
}

void AOFHandler::removeTempFile() {
    char temp[1024];
    snprintf(temp, sizeof(temp), "temp-rewriteaof-bg-%d.aof", this->childPid);
    unlink(temp);
}

void AOFHandler::backgroundFsync() {
    coordinator->getBioHandler()->createBackgroundJob(
            BIO_AOF_FSYNC,
            reinterpret_cast<void*>(this->fd),
            NULL,
            NULL);
}

void AOFHandler::doRealWrite() {
    AbstractFlyServer *flyServer = this->coordinator->getFlyServer();

    /** 写入 */
    ssize_t written = -1;
    written = write(this->fd, this->buf.c_str(), this->buf.length());

    // todo: latency add sample

    bool canlog = false;
    /** 数据没有完全写入*/
    if (written != this->buf.length()) {
        static time_t lastWriteErrorLogTime = 0;
        if (flyServer->getNowt() - lastWriteErrorLogTime
            > AOF_WRITE_LOG_ERROR_RATE) {
            canlog = true;
            lastWriteErrorLogTime = flyServer->getNowt();
        }

        if (-1 == written) {
            if (canlog) {
                coordinator->getLogHandler()->logWarning(
                        "Error writing to the AOF file: %s", strerror(errno));
            }
            this->lastWriteError = errno;
        } else {
            if (canlog) {
                coordinator->getLogHandler()->logWarning(
                        "Short write while writing to "
                        "the AOF file: (nwritten = %lld, "
                        "expected = %lld)",
                        written,
                        this->buf.length());
            }

            /** 裁剪aof文件，将部分写入的数据裁剪掉 */
            if (-1 == ftruncate(this->fd, this->currentSize)) {
                if (canlog) {
                    coordinator->getLogHandler()->logWarning(
                            "Could not remove short write "
                            "from the append-only file.  Redis may refuse "
                            "to load the AOF the next time it starts.  "
                            "ftruncate: %s", strerror(errno));
                }

                /** ENOSPC: 空间不足 */
                this->lastWriteError = ENOSPC;
            } else {
                /** 裁剪成功了，说明一丢丢都没有写入 */
                written = -1;
            }
        }

        /**
         * 如果是AOF_FSYNC_ALWAYS, 回复信息都已经传递给了client的output buffer，
         * 当通知了client，则代表已经做完了fsync，不能裁剪了。
         * todo: 后续研究
         **/
        if (AOF_FSYNC_ALWAYS == this->fsyncStragy) {
            coordinator->getLogHandler()->logWarning(
                    "Can't recover from AOF write error when "
                    "the AOF fsync policy is 'always'. Exiting...");
            exit(1);
        }

        /**
         * 对于没有正确写入的情况，将lastWriteStatus标记为-1，
         * 这样在serverCron中等待下次再继续写入, 本次不执行后续fsync
         **/
        this->lastWriteStatus = -1;
        /** 对于无法裁剪（恢复）的部分写入，裁剪buf */
        if (written > 0) {
            this->currentSize += written;
            this->buf = this->buf.substr(written);
        }
    } else { /** 完全写入了 */
        if (-1 == this->lastWriteStatus) {
            coordinator->getLogHandler()->logWarning(
                    "AOF write error looks solved, can write again.");
            this->lastWriteStatus = 0;
        }
    }

    this->currentSize += written;

    /** 清空aof buf */
    this->buf.clear();
}

void AOFHandler::doRealFsync(bool syncInProgress) {
    AbstractFlyServer *flyServer = this->coordinator->getFlyServer();

    /**
     * 如果配置了no fsync on write或者当前有子进程在进行aof或fdb,
     * 则直接返回，不进行fsync
     **/
    if (this->noFsyncOnRewrite
        || this->haveChildPid()
        || coordinator->getFdbHandler()->haveChildPid()) {
        return;
    }

    /**
     * fsync operation: 如果满足下述两个条件中的一个，则会执行fsync操作
     *  1.是always
     *  2.every second并且当前时间-上次执行fsync时间 > 1秒
     *  对于2执行的是background fsync（将fsync操作放入bio中）
     * */
    if (AOF_FSYNC_ALWAYS == this->fsyncStragy) {
        aof_fsync(this->fd);
    } else if (AOF_FSYNC_EVERYSEC && flyServer->getNowt() > this->lastFsync) {
        /**
         * 如果没有background fsync，则添加一个background fsync操作,
         * 如果有background fsync，则不必添加，减少多余的fsync操作
         **/
        if (!syncInProgress) {
            this->backgroundFsync();
        }
    }
    this->lastFsync = flyServer->getNowt();
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

bool AOFHandler::IsStateOff() const {
    return this->state == AOF_OFF;
}

bool AOFHandler::IsStateWaitRewrite() const {
    return this->state == AOF_WAIT_REWRITE;
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

bool AOFHandler::sizeMeetRewriteCondition() {
    if (this->currentSize < this->rewriteMinSize) {
        return false;
    }

    uint64_t base = this->rewriteBaseSize ? rewriteBaseSize : 1;
    uint64_t growth = (this->currentSize * 100 / base) - 100;
    if (this->rewritePerc != 0 && growth >= this->rewritePerc) {
        return true;
    } else {
        return false;
    }
}

void AOFHandler::setRewritePerc(uint8_t rewritePerc) {
    this->rewritePerc = rewritePerc;
}

void AOFHandler::setRewriteMinSize(off_t rewriteMinSize) {
    this->rewriteMinSize = rewriteMinSize;
}

void AOFHandler::setNoFsyncOnRewrite(bool noFsyncOnRewrite) {
    this->noFsyncOnRewrite = noFsyncOnRewrite;
}

void AOFHandler::setLoadTruncated(bool loadTruncated) {
    this->loadTruncated = loadTruncated;
}

bool AOFHandler::flushPostponed() const {
    return 0 != this->flushPostponedStart;
}

bool AOFHandler::lastWriteHasError() const {
    return -1 == this->lastWriteStatus;
}

int AOFHandler::getFd() const {
    return this->fd;
}

uint64_t AOFHandler::getRewriteBufSize() {
    uint64_t size = 0;
    for (auto iter : this->rewriteBufBlocks) {
        size += iter->used;
    }

    return size;
}

void AOFHandler::clearRewriteBuffer() {
    for (auto iter : this->rewriteBufBlocks) {
        delete iter;
    }

    this->rewriteBufBlocks.clear();
}

void AOFHandler::clearFileEvent() {
    this->coordinator->getEventLoop()->deleteFileEvent(
            this->coordinator->getAofDataPipe()->getWritePipe(),
            ES_WRITABLE);
    this->coordinator->getEventLoop()->deleteFileEvent(
            this->coordinator->getAofDataPipe()->getReadPipe(),
            ES_READABLE);
}

/**
 * 将rewrite buf block中的所有数据都写入aof file中，
 **/
ssize_t AOFHandler::rewriteBufferWriteToFile(int fd) {
    ssize_t count = 0;

    for (auto iter : this->rewriteBufBlocks) {
        /** onceWriten记录当前该block已经写入了多少 */
        int totalWriten = 0;
        while (1) {
            int written = write(fd,
                                iter->buf + totalWriten,
                                iter->used - totalWriten);
            /** 写入失败 */
            if (written <= 0) {
                if (0 == written) {
                    errno = EIO;
                }

                iter->used -= totalWriten;
                iter->free += totalWriten;
                memmove(iter->buf,
                        iter->buf + totalWriten,
                        iter->used - totalWriten);
                return -1;
            }

            count += written;
            totalWriten += written;
            /** 如果已经全部写入，删除该block */
            if (totalWriten == iter->used) {
                this->popFrontRewriteBufBlock();
                break;
            }
        }
    }

    return count;
}
