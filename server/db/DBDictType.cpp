//
// Created by 赵立伟 on 2018/10/13.
//

#include "DBDictType.h"
#include "../dataStructure/dict/Dict.h"
#include "../dataStructure/flyObj/FlyObj.h"

uint64_t DBDictType::hashFunction(const void *key) const {
    if (NULL == key) {
        return 0;
    }

    return dictStrHash(key);
}

void* DBDictType::keyDup(void *key) const {
    return key;
}

void* DBDictType::valDup(void *obj) const {
    return obj;
}

int DBDictType::keyCompare(const void *key1, const void *key2) const {
    if (NULL == key1 && NULL == key2) {
        return 0;
    }
    if (NULL == key1) {
        return -1;
    }
    if (NULL == key2) {
        return 1;
    }

    return dictStrKeyCompare(key1, key2);
}

void DBDictType::keyDestructor(void *key) const {
    if (NULL == key) {
        return;
    }

    return dictStrDestructor(key);
}

void DBDictType::valDestructor(void *obj) const {
    if (NULL == obj) {
        return;
    }

    FlyObj* flyObj = reinterpret_cast<FlyObj *>(obj);
    flyObj->decrRefCount();
}
