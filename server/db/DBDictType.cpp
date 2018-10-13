//
// Created by 赵立伟 on 2018/10/13.
//

#include "DBDictType.h"
#include "../dataStructure/dict/Dict.h"

uint64_t DBDictType::hashFunction(const void *key) const {
    return dictStrHash(key);
}

void* DBDictType::keyDup(void *key) const {
    return key;
}

void* DBDictType::valDup(void *obj) const {
    return obj;
}

int DBDictType::keyCompare(const void *key1, const void *key2) const {
    return dictStrKeyCompare(key1, key2);
}

void DBDictType::keyDestructor(void *key) const {
    return dictStrDestructor(key);
}

void DBDictType::valDestructor(void *obj) const {
    // todo(zlw) FlyObj;
}
