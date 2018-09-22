//
// Created by 赵立伟 on 2018/9/22.
//

#ifndef FLYDB_DICTDEF_H
#define FLYDB_DICTDEF_H

typedef void (*scanProc)(void* priv, void* key, void* val);

const int HASH_TABLE_INITIAL_SIZE = 4;      // hash table初始大小
const int NEED_REHASH_RATIO = 5;            // rehash的触发比例

class DictType {
public:
    virtual unsigned long hashFunction(const void *key) = 0;
    virtual void* keyDup(const void *key) = 0;
    virtual void* valDup(const void *obj) = 0;
    virtual int keyCompare(const void *key1, const void *key2) = 0;
    virtual void keyDestructor(void *key) = 0;
    virtual void valDestructor(void *obj) = 0;
};

#endif //FLYDB_DICTDEF_H
