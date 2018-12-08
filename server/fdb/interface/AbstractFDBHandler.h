//
// Created by 赵立伟 on 2018/12/8.
//

#ifndef FLYDB_ABSTRACTFDBHANDLER_H
#define FLYDB_ABSTRACTFDBHANDLER_H

#include "../FDBDef.h"
#include "../../io/interface/Fio.h"

class AbstractFDBHandler {
public:

    virtual int load(FDBSaveInfo &fdbSaveInfo) = 0;

    virtual int loadFromFio(Fio *fio, FDBSaveInfo &saveInfo) = 0;
};

#endif //FLYDB_ABSTRACTFDBHANDLER_H
