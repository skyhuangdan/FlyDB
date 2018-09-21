//
// Created by 赵立伟 on 2018/9/20.
//

#ifndef FLYDB_DICTTYPE_H
#define FLYDB_DICTTYPE_H

class DictType {
public:
    virtual unsigned int hashFunction(const void *key) = 0;
    virtual void* keyDup(const void *key) = 0;
    virtual void* valDup(const void *obj) = 0;
    virtual int keyCompare(const void *key1, const void *key2) = 0;
    virtual void keyDestructor(void *key) = 0;
    virtual void valDestructor(void *obj) = 0;
};

#endif //FLYDB_DICTTYPE_H
