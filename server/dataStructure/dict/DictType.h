//
// Created by 赵立伟 on 2018/9/24.
//

#ifndef FLYDB_DICTTYPE_H
#define FLYDB_DICTTYPE_H

class DictType {
public:
    virtual unsigned long hashFunction(const void *key) const = 0;
    virtual void* keyDup(const void *key) const = 0;
    virtual void* valDup(const void *obj) const = 0;
    virtual int keyCompare(const void *key1, const void *key2) const = 0;
    virtual void keyDestructor(void *key) const = 0;
    virtual void valDestructor(void *obj) const = 0;
};

#endif //FLYDB_DICTTYPE_H
