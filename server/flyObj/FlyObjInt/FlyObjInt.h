//
// Created by 赵立伟 on 2018/12/9.
//

#ifndef FLYDB_FLYOBJINT_H
#define FLYDB_FLYOBJINT_H


#include "../FlyObjDef.h"
#include "../interface/FlyObj.h"

class FlyObjInt : public FlyObj {

public:
    FlyObjInt(void *ptr, FlyObjType type);
    FlyObjInt(FlyObjType type);
};


#endif //FLYDB_FLYOBJINT_H
