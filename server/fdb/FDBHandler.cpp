//
// Created by 赵立伟 on 2018/11/29.
//

#include <cstdio>
#include <cerrno>
#include <sys/param.h>
#include "FDBHandler.h"
#include "../io/FileFio.h"
#include "../log/FileLogHandler.h"
#include "../flyObj/interface/FlyObj.h"
#include "../log/FileLogFactory.h"
#include "../utils/EndianConvTool.h"
#include "../dataStructure/dict/Dict.cpp"
#include "../dataStructure/skiplist/SkipList.cpp"
#include "../dataStructure/intset/IntSet.h"

#define fdbExitReportCorrupt(...) checkThenExit(__LINE__,__VA_ARGS__)

FDBHandler::FDBHandler(const AbstractCoordinator *coordinator,
                       char *filename,
                       uint64_t maxProcessingChunk) {
    this->coordinator = coordinator;
    this->filename = filename;
    this->maxProcessingChunk = maxProcessingChunk;
    this->logHandler = logFactory->getLogger();
    this->cksum = 0;
    this->endianConvTool = EndianConvTool::getInstance();
}

int FDBHandler::save(FDBSaveInfo &fdbSaveInfo) {
    char tmpfile[256];
    char cwd[MAXPATHLEN];

    snprintf(tmpfile, sizeof(tmpfile), "temp-%d.rdb", getpid());
    FILE *fp = fopen(tmpfile, "w");
    if (NULL == fp) {
        char *cwdp = getcwd(cwd, MAXPATHLEN);
        this->logHandler->logWarning(
                "Failed opening the RDB file %s (in server root dir %s)."
                " for saving: %s",
                filename,
                cwdp != NULL ? cwdp : "unknown",
                strerror(errno));
        return -1;
    }

    Fio *fio = new FileFio(fp, this->maxProcessingChunk);
    if (-1 == saveToFio(fio, 0, fdbSaveInfo)) {
        this->logHandler->logWarning("Write error saving DB on disk: %s",
                                     strerror(errno));
        fclose(fp);
        unlink(tmpfile);
        return -1;
    }

    // 刷新至磁盘中
    if (EOF == fflush(fp) || -1 == fsync(fileno(fp)) || EOF == fclose(fp)) {
        this->logHandler->logWarning("Write error saving DB on disk: %s",
                                     strerror(errno));
        fclose(fp);
        unlink(tmpfile);
        return -1;
    }

    if (-1 == rename(tmpfile, this->filename)) {
        char *cwdp = getcwd(cwd, MAXPATHLEN);
        this->logHandler->logWarning(
                "Error moving temp DB file %s on the final "
                "destination %s (in server root dir %s): %s",
                tmpfile,
                filename,
                cwdp != NULL ? cwdp : "unknown",
                strerror(errno));
        unlink(tmpfile);
        return -1;
    }

    return 1;
}

int FDBHandler::saveToFio(Fio *fio, int flag, FDBSaveInfo &saveInfo) {
    char magic[10];
    snprintf(magic, sizeof(magic), "FLYDB%04d", FDB_VERSION);
    if (-1 == fio->write(magic, 9)) {
        this->logHandler->logWarning("error to save FDB_VERSION\n");
        return -1;
    }

    if (-1 == saveInfoAuxFields(fio, flag, saveInfo)) {
        this->logHandler->logWarning("error to save aux fields\n");
        return -1;
    }

    int dbCount = this->coordinator->getFlyServer()->getFlyDBCount();
    for (int i = 0; i < dbCount; i++) {
        AbstractFlyDB *flyDB = this->coordinator->getFlyServer()->getFlyDB(i);

        flyDB->dictScan(fio, scanProc);
    }

    return 1;
}

void FDBHandler::scanProc(void* priv, std::string *key, FlyObj *val) {
    FioAndflyDB *fioAndflyDB = reinterpret_cast<FioAndflyDB *>(priv);
    AbstractFlyDB *flyDB = fioAndflyDB->flyDB;
    uint64_t expireTime = flyDB->getExpire(key);

    flyDB->getCoordinator()->getFdbHandler()->saveKeyValuePair(
            fioAndflyDB->fio, *key, val, expireTime);
}

int FDBHandler::saveKeyValuePair(Fio *fio,
                                 std::string &key,
                                 FlyObj *val,
                                 int64_t expireTime) {
    // 如果是过期键，则存入过期时间
    if (-1 != expireTime) {
        if (expireTime < miscTool->mstime()) {
            return 0;
        }

        if (-1 == saveType(fio, FDB_OPCODE_EXPIRETIME_MS)) {
            return -1;
        }

        if (-1 == saveMillisecondTime(fio, expireTime)) {
            return -1;
        }
    }

    // 存入value类型
    if (-1 == saveType(fio, val->getType())) {
        return -1;
    }

    // 存入key
    if (-1 == saveRawString(fio, key)) {
        return -1;
    }

    if (-1 == saveObject(fio, val)) {
        return -1;
    }

    return 1;
}

ssize_t FDBHandler::saveObject(Fio *fio, FlyObj *obj) {
    ssize_t written = 0;
    ssize_t n = 0;
    if (FLY_TYPE_STRING == obj->getType()) {
        written += saveRawString(
                fio,
                *reinterpret_cast<std::string*>(obj->getPtr()));
    } else if (FLY_TYPE_LIST == obj->getType()) {
        SkipList<std::string> *sl = (SkipList<std::string> *)obj->getPtr();
        sl->getLength();
        if (-1 == (n = saveLen(fio, sl->getLength()))) {
            return -1;
        }
        written += n;
    } else if (FLY_TYPE_HASH == obj->getType()) {
        Dict<std::string, FlyObj> *dict =
                (Dict<std::string, FlyObj> *)obj->getPtr();
        if (-1 == (n = saveLen(fio, dict->size()))) {
            return -1;
        }
        written += n;

    } else if (FLY_TYPE_SET == obj->getType()) {

    }

    return written;
}

int FDBHandler::saveInfoAuxFields(Fio *fio,
                                  int flags,
                                  FDBSaveInfo &saveInfo) {
    int redis_bits = (sizeof(void*) == 8) ? 64 : 32;
    int aof_preamble = (flags & RDB_SAVE_AOF_PREAMBLE) != 0;

    /* Add a few fields about the state when the RDB was created. */
    if (-1 == saveAuxFieldStrStr(fio, "redis-ver", VERSION)) {
        return -1;
    }

    if (-1 == saveAuxFieldStrInt(fio, "redis-bits", redis_bits)) {
        return -1;
    }

    if (-1 == saveAuxFieldStrInt(fio, "ctime", time(NULL))) {
        return -1;
    }

    /* Handle saving options that generate aux fields. */
    if (-1 == saveAuxFieldStrInt(fio,
                                 "repl-stream-db",
                                 saveInfo.replStreamDB)) {
        return -1;
    }

    /*
    if (-1 == saveAuxFieldStrStr(fio,
                                 "repl-id",
                                 this->coordinator->getFlyServer())) {
        return -1;
    }

    if (-1 == saveAuxFieldStrInt(fio, "repl-offset", server.master_repl_offset)) {
        return -1;
    }
    */

    if (-1 == saveAuxFieldStrInt(fio, "aof-preamble", aof_preamble)) {
        return -1;
    }

    return 1;
}

int FDBHandler::saveAuxFieldStrStr(Fio *fio,
                                   const std::string &key,
                                   const std::string &val) {
    return saveAuxField(fio, key, val);
}

int FDBHandler::saveAuxFieldStrInt(Fio *fio,
                                   const std::string &key,
                                   int64_t val) {
    std::string valStr = std::to_string(val);
    return saveAuxField(fio, key, valStr);
}

int FDBHandler::saveAuxField(Fio *fio,
                             const std::string &key,
                             const std::string &val) {
    if (-1 == saveType(fio, FDB_OPCODE_AUX)) {
        return -1;
    }

    if (-1 == saveRawString(fio, key)) {
        return -1;
    }

    if (-1 == saveRawString(fio, val)) {
        return -1;
    }

    return 1;
}

int FDBHandler::saveMillisecondTime(Fio *fio, int64_t t) {
    return fio->write(&t, 8);
}

int FDBHandler::saveType(Fio *fio, unsigned char type) {
    return fio->write(&type, 1);
}

ssize_t FDBHandler::saveRawString(Fio *fio, const std::string &str) {
    ssize_t written = 0;
    ssize_t n;
    if (-1 == (n = saveLen(fio, str.length()))) {
        return -1;
    }
    written += n;

    if (str.length() > 0) {
        if (-1 == fio->write(str.c_str(), str.length())) {
            return -1;
        }
        written += n;
    }

    return written;
}

ssize_t FDBHandler::saveLen(Fio *fio, uint64_t len) {
    unsigned char buf[2];
    size_t nwritten;

    if (len < (1 << 6)) {
        /* Save a 6 bit len */
        buf[0] = (len & 0xFF) | (FDB_6BITLEN << 6);
        if (-1 == fio->write(buf, 1)) {
            return -1;
        }
        nwritten = 1;
    } else if (len < (1<<14)) {
        /* Save a 14 bit len */
        buf[0] = ((len >> 8) & 0xFF) | (FDB_14BITLEN << 6);
        buf[1] = len & 0xFF;
        if (-1 == fio->write(buf, 2)) {
            return -1;
        }
        nwritten = 2;
    } else if (len <= UINT32_MAX) {
        /* Save a 32 bit len */
        buf[0] = FDB_32BITLEN;
        if (-1 == fio->write(buf, 1)) {
            return -1;
        }
        uint32_t len32 = htonl(len);
        if (-1 == fio->write(&len32, 4)) {
            return -1;
        }
        nwritten = 1 + 4;
    } else {
        /* Save a 64 bit len */
        buf[0] = FDB_64BITLEN;
        if (-1 == fio->write(buf, 1)) {
            return -1;
        }

        len = htonll(len);
        if (-1 == fio->write(&len, 8)) {
            return -1;
        }
        nwritten = 1 + 8;
    }

    return nwritten;
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
    int version = 0;
    // 检查FDB文件头部
    if (-1 == (version = checkHeader(fio))) {
        return -1;
    }

    int64_t expireTime;
    AbstractFlyDB *flyDB = coordinator->getFlyServer()->getFlyDB(0);

    while (1) {
        char type = loadChar(fio);

        if (FDB_OPCODE_EXPIRETIME == type) {
            // 获取过期时间
            if (-1 == (expireTime = loadTime(fio))) {
                fdbExitReportCorrupt("Unexpected EOF reading RDB file");
                return -1;
            }
            // exchange to millisecond
            expireTime *= 1000;

            // 重新获取类型字段
            if (-1 == (type = loadChar(fio))) {
                fdbExitReportCorrupt("Unexpected EOF reading RDB file");
                return -1;
            }
        } else if (FDB_OPCODE_EXPIRETIME_MS == type) {
            if (-1 == (expireTime = loadMillisecondTime(fio))) {
                fdbExitReportCorrupt("Unexpected EOF reading RDB file");
                return -1;
            }

            // 重新获取类型字段
            if (-1 == (type = loadChar(fio))) {
                fdbExitReportCorrupt("Unexpected EOF reading RDB file");
                return -1;
            }
        } else if (FDB_OPCODE_RESIZEDB == type) {
            // 读取dbSize和expireSize
            uint64_t dbSize = 0, expireSize = 0;
            if (-1 == (dbSize = loadNum(fio, NULL))) {
                fdbExitReportCorrupt("Unexpected EOF reading RDB file");
                return -1;
            }
            if (-1 == (expireSize = loadNum(fio, NULL))) {
                fdbExitReportCorrupt("Unexpected EOF reading RDB file");
                return -1;
            }

            // 分别为dict和expire扩容
            flyDB->expandDict(dbSize);
            flyDB->expandExpire(expireSize);
            continue;
        } else if (FDB_OPCODE_SELECTDB == type) {
            int64_t dbId = 0;
            if (-1 == (dbId = loadNum(fio, NULL))) {
                fdbExitReportCorrupt("Unexpected EOF reading RDB file");
                return -1;
            }

            AbstractFlyDB *temp = NULL;
            if (NULL != (temp = coordinator->getFlyServer()->getFlyDB(dbId))) {
                flyDB = temp;
            }
            continue;
        } else if (FDB_OPCODE_AUX == type) {
            FlyObj *auxkey = NULL, *auxval = NULL;
            if (NULL ==
                (auxkey = loadStringObject(fio))) {
                fdbExitReportCorrupt("Unexpected EOF reading RDB file");
                return -1;
            }
            if (NULL ==
                (auxval = loadStringObject(fio))) {
                fdbExitReportCorrupt("Unexpected EOF reading RDB file");
                return -1;
            }

            // 如果开头是'%' 说明是information字段，使用notice打日志
            if ('%' == ((std::string *)auxkey->getPtr())->at(0)) {
                this->logHandler->logNotice("FDB '%s': %s",
                                            auxkey->getPtr(),
                                            auxval->getPtr());

            } else if (strcasecmp(((std::string *)auxkey->getPtr())->c_str(),
                                  "repl-stream-db")) {
                saveInfo.replStreamDB =
                        atoi(((std::string *)auxval->getPtr())->c_str());
            } else if (strcasecmp(((std::string *)auxkey->getPtr())->c_str(),
                                  "repl-id")) {
                if (CONFIG_RUN_ID_SIZE ==
                    ((std::string *)auxval->getPtr())->length()) {
                    memcpy(saveInfo.replID,
                           auxval->getPtr(),
                           CONFIG_RUN_ID_SIZE + 1);
                    saveInfo.replIDIsSet = 1;
                }
            } else if (strcasecmp(((std::string *)auxkey->getPtr())->c_str(),
                                  "repl-offset")) {
                saveInfo.replOffset = strtoll(
                        ((std::string *)auxval->getPtr())->c_str(),
                        NULL,
                        10);
            } else {
                this->logHandler->logDebug("Unrecognized RDB AUX field: '%s'",
                                           auxkey->getPtr());
            }

            auxkey->decrRefCount();
            auxval->decrRefCount();
            continue;
        } else if (FDB_OPCODE_EOF == type) {
            break;
        }

        // load key-value, and insert into flydb
        std::string *key = NULL;
        FlyObj *val = NULL;
        if (NULL ==
            (key = loadStringPlain(fio))) {
            fdbExitReportCorrupt("Unexpected EOF reading RDB file");
            return -1;
        }

        if (NULL == (val = loadObject(type, fio))) {
            fdbExitReportCorrupt("Unexpected EOF reading RDB file");
            return -1;
        }

        if (expireTime != -1 && expireTime < miscTool->mstime()) {
            val->decrRefCount();
            continue;
        }

        flyDB->addExpire(key, val, expireTime);
    }

    // check sum
    uint64_t cksum;
    if (0 == fio->read(&cksum, 8)) {
        fdbExitReportCorrupt("Unexpected EOF reading RDB file");
        return -1;
    }
    this->endianConvTool->memrev32ifbe(&cksum);
    if (0 == cksum) {
        this->logHandler->logWarning(
                "RDB file was saved with checksum disabled: "
                "no check performed.");
    } else if (this->cksum != cksum) {
        this->logHandler->logWarning("Wrong RDB checksum. Aborting now.");
        fdbExitReportCorrupt("FDB CRC error");
    }

    return 1;
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

// 返回FlyObj类型数据
FlyObj* FDBHandler::loadStringObject(Fio *fio) {
    return reinterpret_cast<FlyObj*>(
            this->genericLoadStringObject(fio, FDB_LOAD_OBJECT, NULL));
}

// 返回string类型数据
std::string* FDBHandler::loadStringPlain(Fio *fio) {
    return reinterpret_cast<std::string*>(
            this->genericLoadStringObject(fio, FDB_LOAD_STRING, NULL));
}

/**
 * 从FDB中加载string object
 *
 * FDB_LOAD_STRING: 返回string类型数据
 * FDB_LOAD_OBJECT: 返回FlyObj类型数据
 *
 * On I/O error NULL is returned.
 */
void* FDBHandler::genericLoadStringObject(Fio *fio, int flag, size_t *lenptr) {
    int encoded = 0;
    int len;
    if (-1 == (len = loadNum(fio, &encoded))) {
        return NULL;
    }

    // len <= 0
    if (len <= 0) {
        this->logHandler->logWarning("the len <= 0! len = %d", len);
        return NULL;
    }

    // 根据len从fio中读取字符串
    std::string *str = new std::string();
    if (-1 == fio->read((void*)(str->c_str()), len)) {
        this->logHandler->logWarning("error to load string from fio!");
        delete str;
    }

    // 根据flag返回FlyObj或者直接返回string
    if (flag & FDB_LOAD_OBJECT) {
        return this->coordinator->getFlyObjStringFactory()->getObject(str);
    } else {
        return str;
    }
}

void* FDBHandler::loadIntegerObject(Fio *fio,
                                    int encode,
                                    int flag,
                                    size_t *lenptr) {
    unsigned char enc[4];
    long long val;

    switch (encode) {
        case FDB_ENC_INT8:
            if (fio->read(enc, 1) == 0) {
                return NULL;
            }
            val = (signed char) (enc[0]);
            break;
        case FDB_ENC_INT16:
            if (fio->read(enc, 2) == 0) {
                return NULL;
            }
            val = enc[0] | (enc[1] << 8);
            break;
        case FDB_ENC_INT32:
            if (fio->read(enc, 4) == 0) {
                return NULL;
            }
            val = enc[0] | enc[1] << 8 | enc[2] << 16 | enc[3] << 24;
            break;
        default:
            val = 0;
            fdbExitReportCorrupt("Unknown RDB integer encoding type %d", encode);
    }

    void *res = new std::string(std::to_string(val));
    if (FDB_LOAD_OBJECT == flag) {
        res = this->coordinator->getFlyObjStringFactory()->getObject(res);
    }

    return res;
}

/**
 * 暂时先不支持压缩
 */
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
        this->logHandler->logWarning(
                "error to load millisecond time from fio!");
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
    if (-1 == fio->read(&buf, 1)) {
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
        if (-1 == fio->read(&num, 4)) {
            this->logHandler->logWarning("error to load len from fio!");
            return -1;
        }

        // 需要降网络字节序转换成本机字节序
        *numptr = ntohl(num);
    } else if (FDB_64BITLEN == buf) {
        uint64_t len;
        if (-1 == fio->read(&len, 8)) {
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
        return -1;
    }

    return 1;
}

FlyObj* FDBHandler::loadObject(int type, Fio *fio) {
    FlyObj *obj = NULL;
    uint64_t len = 0;

    if (FLY_TYPE_STRING == type) {
        obj = loadStringObject(fio);
    } else if (FLY_TYPE_HASH == type) {
        if (-1 == (len = loadNum(fio, NULL))) {
            return obj;
        }

        Dict<std::string, std::string> *dict =
                new Dict<std::string, std::string>();
        while (len--) {
            std::string *key = NULL, *val = NULL;
            if (NULL == (key = loadStringPlain(fio))) {
                fdbExitReportCorrupt("Unexpected EOF reading RDB file");
                return obj;
            }
            if (NULL == (key = loadStringPlain(fio))) {
                fdbExitReportCorrupt("Unexpected EOF reading RDB file");
                return obj;
            }
            dict->addEntry(key, val);
        }
        obj = coordinator->getFlyObjHashTableFactory()->getObject(dict);
    } else if (FLY_TYPE_LIST == type) {
        if (-1 == (len = loadNum(fio, NULL))) {
            return obj;
        }

        SkipList<std::string> *list = new SkipList<std::string>();
        while (len--) {
            std::string *val = NULL;
            if (NULL == (val = loadStringPlain(fio))) {
                fdbExitReportCorrupt("error to read RDB file");
                return obj;
            }

            list->insertNode(0, val);
        }
        obj = coordinator->getFlyObjLinkedListFactory()->getObject(list);
    } else if (FLY_TYPE_SET == type) {
        if (-1 == (len = loadNum(fio, NULL))) {
            return obj;
        }

        IntSet *intset = new IntSet();
        int num;
        while (len--) {
            if (-1 == (num = loadNum(fio, NULL))) {
                fdbExitReportCorrupt("error to read RDB file");
            }
            intset->add(num);
        }

        obj = coordinator->getFlyObjIntSetFactory()->getObject(intset);
    }

    return obj;
}

int FDBHandler::checkHeader(Fio *fio) {
    // 读取头部字节
    char buf[1024];
    if (0 == fio->read(buf, 9)) {
        logHandler->logWarning(
                "Short read or OOM loading DB. Unrecoverable error, "
                "aborting now.");
        fdbExitReportCorrupt("error to load header!");
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

    return version;
}

void FDBHandler::checkThenExit(int linenum, char *reason, ...) {

}
