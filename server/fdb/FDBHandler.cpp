//
// Created by 赵立伟 on 2018/11/29.
//

#include <cstdio>
#include <cerrno>
#include "FDBHandler.h"
#include "../io/FileFio.h"
#include "../log/LogHandler.h"

FDBHandler::FDBHandler(FlyServer *flyServer,
                       char *filename,
                       uint64_t maxProcessingChunk) {
    this->flyServer = flyServer;
    this->filename = filename;
    this->maxProcessingChunk = maxProcessingChunk;
    this->logHandler = LogHandler::getInstance();
}

int FDBHandler::load(FDBSaveInfo &fdbSaveInfo) {
    // open fdb file with read premission
    FILE *fp;
    if (NULL == (fp = fopen(this->filename, "r"))) {
        return -1;
    }

    // start to load
    this->startToLoad();

    // do real load
    int res = loadFromFile(fp, fdbSaveInfo);

    // 读取完毕
    fclose(fp);
    this->stopLoad();

    return res;
}

int FDBHandler::loadFromFile(FILE *fp, FDBSaveInfo &saveInfo) {
    Fio *fio = new FileFio(fp, this->maxProcessingChunk);
    int res = loadFromFio(fio, saveInfo);
    delete fio;

    return res;
}

int FDBHandler::loadFromFio(Fio *fio, FDBSaveInfo &saveInfo) {
    // 检查FDB文件头部
    if (-1 == checkHeader(fio)) {
        return -1;
    }

    uint64_t expireTime;
    FlyDB *flyDB = flyServer->getFlyDB(0);

    while (1) {
        char type = loadChar(fio);
        switch (type) {
            case FDB_OPCODE_EXPIRETIME:
                // 获取过期时间
                if (-1 == (expireTime = loadTime(fio))) {
                    goto err;
                }
                // exchange to millisecond
                expireTime *= 1000;

                // 重新获取类型字段
                if (-1 == (type = loadChar(fio))) {
                    goto err;
                }
                break;
            case FDB_OPCODE_EXPIRETIME_MS:
                if (-1 == (expireTime = loadMillisecondTime(fio))) {
                    goto err;
                }

                // 重新获取类型字段
                if (-1 == (type = loadChar(fio))) {
                    goto err;
                }
                break;
            case FDB_OPCODE_EOF:
                // todo: load completed
                return 1;
            case FDB_OPCODE_SELECTDB:
                continue;
            case FDB_OPCODE_RESIZEDB:
                break;
            case FDB_OPCODE_AUX:
                break;
        }

    }
err:
    logHandler->logWarning("Short read or OOM loading DB. "
                           "Unrecoverable error, aborting now.");
    // todo: rdbExitReportCorruptRDB
    return -1;

}

void FDBHandler::startToLoad() {
    this->loading = true;
    this->loadingStartTime = time(NULL);
}

void FDBHandler::stopLoad() {
    this->loading = false;
}

char FDBHandler::loadChar(Fio *fio) {
    char ch;
    if (-1 == fio->read(&ch, 1)) {
        this->logHandler->logWarning("error to load char from fio!");
        return -1;
    }

    return ch;
}

time_t FDBHandler::loadTime(Fio *fio) {
    uint32_t res;
    if (-1 == fio->read(&res, 4)) {
        this->logHandler->logWarning("error to load time from fio!");
        return -1;
    }

    return res;
}

uint64_t FDBHandler::loadMillisecondTime(Fio *fio) {
    uint64_t res;
    if (-1 == fio->read(&res, 8)) {
        this->logHandler->logWarning("error to load millisecond time from fio!");
        return -1;
    }

    return res;
}

int FDBHandler::checkHeader(Fio *fio) {
    // 读取头部字节
    char buf[1024];
    if (0 == fio->read(buf, 9)) {
        logHandler->logWarning("Short read or OOM loading DB. Unrecoverable error, aborting now.");
        // todo rdbExitReportCorruptRDB
        return -1;
    }

    buf[9] = '\0';
    if (memcpy(buf, "FLYDB", 5) != 0) {
        logHandler->logWarning("Wrong signature trying to load DB from file");
        errno = EINVAL;
        return -1;
    }

    int version = atoi(buf + 5);
    if (version < 1 || version > FDB_VERSION) {
        logHandler->logWarning("Can't handle FDB format version %d", version);
        errno = EINVAL;
        return -1;
    }

    return 1;
}