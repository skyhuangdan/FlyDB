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

        if (FDB_OPCODE_EXPIRETIME == type) {
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
        } else if (FDB_OPCODE_EXPIRETIME_MS == type) {
            if (-1 == (expireTime = loadMillisecondTime(fio))) {
                goto err;
            }

            // 重新获取类型字段
            if (-1 == (type = loadChar(fio))) {
                goto err;
            }
        } else if (FDB_OPCODE_RESIZEDB == type) {
            // 读取dbSize和expireSize
            uint64_t dbSize = 0, expireSize = 0;
            if (-1 == (dbSize = loadNum(fio, NULL))) {
                goto err;
            }
            if (-1 == (expireSize = loadNum(fio, NULL))) {
                goto err;
            }

            // 分别为dict和expire扩容
            flyDB->expandDict(dbSize);
            flyDB->expandExpire(expireSize);
            break;
        } else if (FDB_OPCODE_SELECTDB == type) {
            int64_t dbId = 0;
            if (-1 == (dbId = loadNum(fio, NULL))) {
                goto err;
            }

            FlyDB *temp = NULL;
            if (NULL != (temp = flyServer->getFlyDB(dbId))) {
                flyDB = temp;
            }
            continue;
        } else if (FDB_OPCODE_AUX == type) {

            continue;
        } else if (FDB_OPCODE_EOF == type) {
            return 1;
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

/**
 * 从FDB中加载string object
 *
 * RDB_LOAD_STRING: 返回string类型数据
 * RDB_LOAD_OBJECT: 返回FlyObj类型数据
 *
 * On I/O error NULL is returned.
 */
void* FDBHandler::genericLoadStringObject(Fio *fio, int flag, size_t *lenptr) {
    int encoded = 0;
    int len;
    if (-1 == loadNum(fio, &encoded)) {
        return NULL;
    }

    if (0 != encoded) {
        switch (len) {
            case FDB_ENC_INT8:
            case FDB_ENC_INT16:
            case FDB_ENC_INT32:
                return loadIntegerObject(fio, len, lenptr);
            case FDB_ENC_LZF:
                return loadLzfStringObject(fio, flag, lenptr);
            default:
                int i;
                // todo :rdbExitReportCorruptRDB
        }
    }

}

void* FDBHandler::loadIntegerObject(Fio *fio, int flag, size_t *lenptr) {

}

void* FDBHandler::loadLzfStringObject(Fio *fio, int flag, size_t *lenptr) {

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

int FDBHandler::loadNum(Fio *fio, int *encoded) {
    uint64_t num = 0;
    if (-1 == loadNumByRef(fio, encoded, &num)) {
        return -1;
    }

    return num;
}

int FDBHandler::loadNumByRef(Fio *fio, int *encoded, uint64_t *numptr) {
    char buf;
    if(-1 == fio->read(&buf, 1)) {
        this->logHandler->logWarning("error to load len from fio!");
        return -1;
    }

    char temp = (buf & 0xC0) >> 6;
    if (FDB_6BITLEN == temp) {
        *numptr = buf & 0x3F;
    } else if (FDB_14BITLEN == temp) {
        // 读取低8位
        char low;
        if(-1 == fio->read(&low, 1)) {
            this->logHandler->logWarning("error to load len from fio!");
            return -1;
        }

        *numptr = (buf & 0x3F << 8) + low;
    } else if (FDB_32BITLEN == buf) {
        uint32_t num;
        if(-1 == fio->read(&num, 4)) {
            this->logHandler->logWarning("error to load len from fio!");
            return -1;
        }

        // 需要降网络字节序转换成本机字节序
        *numptr = ntohl(num);
    } else if (FDB_64BITLEN == buf) {
        uint64_t len;
        if(-1 == fio->read(&len, 8)) {
            this->logHandler->logWarning("error to load len from fio!");
            return -1;
        }

        // 需要降网络字节序转换成本机字节序
        *numptr = ntohll(len);
    } else if (FDB_ENCVAL == buf) {     // 令encoded=1表示自定义类型
        if (NULL != encoded) {
            *encoded = 1;
        }
        *numptr = buf & 0x3F;
    } else {
        // todo: rdbExitReportCorruptRDB
        return -1;
    }

    return 1;
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