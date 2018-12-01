//
// Created by 赵立伟 on 2018/11/29.
//

#ifndef FLYDB_FDBHANDLER_H
#define FLYDB_FDBHANDLER_H

#include <ctime>
#include <cstdint>
#include "FDBDef.h"
#include "../io/Fio.h"

class LogHandler;
class FDBHandler {
public:
    FDBHandler(char *filename, uint64_t maxProcessingChunk);
    int load(FDBSaveInfo &fdbSaveInfo);
    int loadFromFio(Fio *fio, FDBSaveInfo &saveInfo);

private:
    void startToLoad();
    int loadFromFile(FILE *fp, FDBSaveInfo &fdbSaveInfo);
    void stopLoad();
    char loadChar(Fio *fio);
    int checkHeader(Fio *fio);

    char *filename;
    bool loading = false;
    time_t loadingStartTime = 0;
    uint64_t loadBytes = 0;
    uint64_t loadTotalBytes = 0;
    off_t maxProcessingChunk = 0;
    
    LogHandler *logHandler;

};


#endif //FLYDB_FDBHANDLER_H
