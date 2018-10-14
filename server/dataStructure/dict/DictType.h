//
// Created by 赵立伟 on 2018/9/24.
//

#ifndef FLYDB_DICTTYPE_H
#define FLYDB_DICTTYPE_H

#include <cstdint>

class DictType {
public:
    virtual uint64_t hashFunction(const void *key) const;
    virtual void* keyDup(void *key) const;
    virtual void* valDup(void *obj) const;
    virtual int keyCompare(const void *key1, const void *key2) const;
    virtual void keyDestructor(void *key) const;
    virtual void valDestructor(void *obj) const;
};

#endif //FLYDB_DICTTYPE_H
