//
// Created by 赵立伟 on 2018/12/9.
//

#ifndef FLYDB_FLYOBJHASHTABLE_H
#define FLYDB_FLYOBJHASHTABLE_H

#include "../interface/FlyObj.h"

class FlyObjHashTable : public FlyObj {

public:
    FlyObjHashTable(FlyObjType type);
    FlyObjHashTable(void *ptr, FlyObjType type);
    void decrRefCount();
    FlyObjEncode getEncode();
};


#endif //FLYDB_FLYOBJHASHTABLE_H
