//
// Created by 赵立伟 on 2018/11/29.
//

#ifndef FLYDB_FDBHANDLER_H
#define FLYDB_FDBHANDLER_H

#include <ctime>
#include <cstdint>
#include "FDBDef.h"
#include "../io/interface/Fio.h"

class AbstractLogHandler;
class FlyServer;

class FDBHandler {
public:
    FDBHandler(FlyServer *flyServer,
               char *filename,
               uint64_t maxProcessingChunk);
    int load(FDBSaveInfo &fdbSaveInfo);
    int loadFromFio(Fio *fio, FDBSaveInfo &saveInfo);

private:
    void startToLoad();
    int loadFromFile(FILE *fp, FDBSaveInfo &fdbSaveInfo);
    void stopLoad();
    char loadChar(Fio *fio);
    time_t loadTime(Fio *fio);
    uint64_t loadMillisecondTime(Fio *fio);
    int loadNum(Fio *fio, int *encoded);
    int loadNumByRef(Fio *fio, int *encoded, uint64_t *lenptr);
    void* loadStringObject(Fio *fio, int flag, size_t *lenptr);
    void* loadStringPlain(Fio *fio, int flag, size_t *lenptr);
    void* genericLoadStringObject(Fio *fio, int flag, size_t *lenptr);
    void* loadIntegerObject(Fio *fio, int flag, size_t *lenptr);
    void* loadLzfStringObject(Fio *fio, int flag, size_t *lenptr);
    int checkHeader(Fio *fio);
    void checkThenExit(int linenum, char *reason, ...);

    char *filename;
    bool loading = false;
    time_t loadingStartTime = 0;
    uint64_t loadBytes = 0;
    uint64_t loadTotalBytes = 0;
    off_t maxProcessingChunk = 0;

    AbstractLogHandler *logHandler;
    FlyServer *flyServer;
};


#endif //FLYDB_FDBHANDLER_H
