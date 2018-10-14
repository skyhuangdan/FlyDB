//
// Created by 赵立伟 on 2018/10/14.
//

#include <cstdint>
#include "DictType.h"

uint64_t DictType::hashFunction(const void *key) const {
    return 0;
}

void* DictType::keyDup(void *key) const {
    return key;
}

void* DictType::valDup(void *obj) const {
    return obj;
}

int DictType::keyCompare(const void *key1, const void *key2) const {
    return 0;
}

void DictType::keyDestructor(void *key) const {
    return;
}

void DictType::valDestructor(void *obj) const {
    return;
}
