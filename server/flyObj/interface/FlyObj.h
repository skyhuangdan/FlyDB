//
// Created by 赵立伟 on 2018/10/13.
//

#ifndef FLYDB_FLYOBJ_H
#define FLYDB_FLYOBJ_H

#include <cstdint>
#include "../FlyObjDef.h"

class FlyObj {
public:
    FlyObj(FlyObjType type);
    FlyObj(void *ptr, FlyObjType type);
    virtual ~FlyObj();
    void incrRefCount();
    void decrRefCount();
    void resetRefCount();
    FlyObjType getType() const;
    void setType(FlyObjType type);
    uint32_t getRefCount() const;
    void setRefCount(uint32_t refCount);
    uint16_t getLru() const;
    void setLru(uint16_t lru);
    void *getPtr() const;
    void setPtr(void *ptr);

protected:
    FlyObjType type;
    uint32_t refCount;
    uint16_t lru;
    void* ptr;
};

#endif //FLYDB_FLYOBJ_H
