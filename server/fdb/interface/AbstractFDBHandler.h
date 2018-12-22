//
// Created by 赵立伟 on 2018/12/8.
//

#ifndef FLYDB_ABSTRACTFDBHANDLER_H
#define FLYDB_ABSTRACTFDBHANDLER_H

#include "../FDBDef.h"
#include "../../io/interface/Fio.h"

class FlyObj;

class AbstractFDBHandler {
public:

    virtual int load(FDBSaveInfo &fdbSaveInfo) = 0;

    virtual int loadFromFio(Fio *fio, FDBSaveInfo &saveInfo) = 0;

    virtual int save(FDBSaveInfo &fdbSaveInfo) = 0;

    virtual int saveKeyValuePair(Fio *fio,
                                 std::string &key,
                                 FlyObj *val,
                                 int64_t expireTime) = 0;

    virtual ssize_t saveRawString(Fio *fio, const std::string &str) = 0;
};

#endif //FLYDB_ABSTRACTFDBHANDLER_H
