//
// Created by 赵立伟 on 2018/11/29.
//

#include <fcntl.h>
#include "AOFHandler.h"
#include "../io/FileFio.h"

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
     * 无论是schedule运行还是直接运行完，都是等待rewrite完毕，
     * 以便于之后执行append data on disk
     **/
    this->state = AOF_WAIT_REWRITE;
    return 1;
}

int AOFHandler::rewriteBackground() {
    if (coordinator->getFdbHandler()->haveChildPid()
        || this->haveChildPid()) {
        return -1;
    }

    /** 打开主线程与子线程之间的管道 */
    if (-1 == coordinator->openAllPipe()) {
        return -1;
    }
    if (-1 == coordinator->getAofDataPipe()->setReadNonBlock()
        || -1 == coordinator->getAofDataPipe()->setWriteNonBlock()
        || -1 == coordinator->getChildInfoPipe()->setReadNonBlock()) {
        coordinator->closeAllPipe();
        return -1;
    }

    /** 创建文件事件: 用于child-->parent ack的读取处理 */
    if (-1 == this->coordinator->getEventLoop()->createFileEvent(
            coordinator->getAofAckToParentPipe()->getReadPipe(),
            ES_READABLE,
            NULL,
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
            coordinator->getChildInfoPipe()->sendInfo(PIPE_TYPE_AOF, 0);
            exit(0);
        }

        /** write failure */
        exit(1);
    } else {
        /** parent */
        if (-1 == childPid) {    /** 创建子线程失败 */
            coordinator->getAofDataPipe()->closeAll();
            coordinator->getAofAckToChildPipe()->closeAll();
            coordinator->getAofAckToParentPipe()->closeAll();
            coordinator->getChildInfoPipe()->closeAll();
        }

        this->childPid = childPid;
        this->scheduled = false;
        // todo: updateDictResizePolicy

    }

    return 1;
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

    /** write aof diff data which is from parent to aof file*/
    return this->rewriteAppendOnlyFileDiff(tmpfile, fio);
}

int AOFHandler::rewriteAppendOnlyFileDiff(char *tmpfile, FileFio *fio) {
    FILE *fp = fio->getFp();
    /**
     * 从parent读取持久化之后的aof数据
     *      只要一直有从parent到本线程的数据，那尽量多读一段时间（1000ms）
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

int AOFHandler::rewriteAppendOnlyFileFio(Fio *fio) {

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

