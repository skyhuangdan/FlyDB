//
// Created by 赵立伟 on 2018/10/13.
//

#ifndef FLYDB_FLYOBJ_H
#define FLYDB_FLYOBJ_H

#include <cstdint>
#include "encoding/FlyObjEncoding.h"

class FlyObj {
public:
    FlyObj(void *ptr, FlyObjType type);
    void incrRefCount();
    void decrRefCount();
    void resetRefCount();
    FlyObjType getType() const;
    void setType(FlyObjType type);
    const FlyObjEncoding *getEncoding() const;
    void setEncoding(const FlyObjEncoding *encoding);
    uint32_t getRefCount() const;
    void setRefCount(uint32_t refCount);
    uint16_t getLru() const;
    void setLru(uint16_t lru);
    void *getPtr() const;
    void setPtr(void *ptr);

private:
    FlyObjType type;
    const FlyObjEncoding* encoding;
    uint32_t refCount;
    uint16_t lru;
    void* ptr;
};


#endif //FLYDB_FLYOBJ_H
