//
// Created by 赵立伟 on 2018/10/13.
//

#ifndef FLYDB_DBDICTTYPE_H
#define FLYDB_DBDICTTYPE_H

#include "../dataStructure/dict/DictType.h"

class DBDictType : public DictType {
public:
    uint64_t hashFunction(const void *key) const;
    void* keyDup(void *key) const;
    void* valDup(void *obj) const;
    int keyCompare(const void *key1, const void *key2) const;
    void keyDestructor(void *key) const;
    void valDestructor(void *obj) const;
};


#endif //FLYDB_DBDICTTYPE_H
