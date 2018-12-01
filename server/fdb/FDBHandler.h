//
// Created by 赵立伟 on 2018/11/29.
//

#ifndef FLYDB_FDBHANDLER_H
#define FLYDB_FDBHANDLER_H

#include <ctime>
#include <cstdint>
#include "FDBDef.h"
#include "../io/Fio.h"

class FDBHandler {
public:
    FDBHandler(char *filename);
    int load(fdbSaveInfo &fdbSaveInfo);

private:
    void startToLoad();
    void doRealLoad(Fio *fio, fdbSaveInfo &fdbSaveInfo);
    void stopLoad();

    char *filename;
    bool loading = false;
    time_t loadingStartTime = 0;
    uint64_t loadBytes = 0;
    uint64_t loadTotalBytes = 0;

};


#endif //FLYDB_FDBHANDLER_H
