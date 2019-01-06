//
// Created by 赵立伟 on 2018/12/9.
//

#ifndef FLYDB_FLYOBJINTSET_H
#define FLYDB_FLYOBJINTSET_H


#include "../FlyObjDef.h"
#include "../interface/FlyObj.h"

class FlyObjIntSet : public FlyObj {

public:
    FlyObjIntSet(FlyObjType type);
    FlyObjIntSet(void *ptr, FlyObjType type);
};


#endif //FLYDB_FLYOBJINTSET_H
