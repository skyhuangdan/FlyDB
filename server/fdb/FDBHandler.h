//
// Created by 赵立伟 on 2018/11/29.
//

#ifndef FLYDB_FDBHANDLER_H
#define FLYDB_FDBHANDLER_H

#include <ctime>
#include <cstdint>
#include "FDBDef.h"
#include "../io/interface/Fio.h"
#include "interface/AbstractFDBHandler.h"
#include "../coordinator/interface/AbstractCoordinator.h"
#include "../log/interface/AbstractLogHandler.h"
#include "../utils/EndianConvTool.h"

class FDBHandler : public AbstractFDBHandler {
public:
    FDBHandler(const AbstractCoordinator *coordinator,
               char *filename,
               uint64_t maxProcessingChunk);
    int load(FDBSaveInfo &fdbSaveInfo);
    int save(FDBSaveInfo &fdbSaveInfo);

private:
    static void scanProc(void* priv, std::string *key, FlyObj *val);
    int saveToFio(Fio *fio, int flag, FDBSaveInfo &saveInfo);
    size_t saveLen(Fio *fio, uint64_t len);
    int saveInfoAuxFields(Fio *fio,
                          int flags,
                          FDBSaveInfo &saveInfo);
    int saveAuxFieldStrStr(Fio *fio,
                           const std::string &key,
                           const std::string &val);
    int saveAuxFieldStrInt(Fio *fio,
                           const std::string &key,
                           int64_t val);
    int saveAuxField(Fio *fio,
                     const std::string &key,
                     const std::string &val);
    int saveType(Fio *fio, unsigned char type);
    ssize_t saveRawString(Fio *fio, const std::string &str);
    int loadFromFio(Fio *fio, FDBSaveInfo &saveInfo);
    void startToLoad();
    int loadFromFile(FILE *fp, FDBSaveInfo &fdbSaveInfo);
    void stopLoad();
    char loadChar(Fio *fio);
    time_t loadTime(Fio *fio);
    uint64_t loadMillisecondTime(Fio *fio);
    int loadNum(Fio *fio, int *encoded);
    int loadNumByRef(Fio *fio, int *encoded, uint64_t *lenptr);
    FlyObj* loadStringObject(Fio *fio);
    std::string* loadStringPlain(Fio *fio);
    void* genericLoadStringObject(Fio *fio, int flag, size_t *lenptr);
    void* loadIntegerObject(Fio *fio, int encoding, int flag, size_t *lenptr);
    void* loadLzfStringObject(Fio *fio, int flag, size_t *lenptr);
    int checkHeader(Fio *fio);
    void checkThenExit(int linenum, char *reason, ...);
    FlyObj* loadObject(int type, Fio *fio);

    char *filename;
    bool loading = false;
    time_t loadingStartTime = 0;
    uint64_t loadBytes = 0;
    uint64_t loadTotalBytes = 0;
    off_t maxProcessingChunk = 0;
    uint64_t cksum = 0;

    AbstractLogHandler *logHandler;
    const AbstractCoordinator *coordinator;
    EndianConvTool *endianConvTool;
};


#endif //FLYDB_FDBHANDLER_H
