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

    /** todo: rewrite */
    off_t autosync = this->rewriteIncrementalFsync ? AOF_AUTOSYNC_BYTES : 0;
    FileFio* fio = FileFio::Builder().file(fp).autosync(autosync).build();

    /** rename file to real file name */
    if (-1 == rename(tmpfile, this->fileName)) {
        this->coordinator->getLogHandler()->logWarning(
                "Error moving temp append only file "
                "on the final destination: %s", strerror(errno));
        return -1;
    }
    this->coordinator->getLogHandler()->logNotice(
            "SYNC append only file rewrite performed");
    return 1;

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

void AOFHandler::setFsync(int fsync) {
    this->fsync = fsync;
}

void AOFHandler::setRewriteIncrementalFsync(bool rewriteIncrementalFsync) {
    this->rewriteIncrementalFsync = rewriteIncrementalFsync;
}

