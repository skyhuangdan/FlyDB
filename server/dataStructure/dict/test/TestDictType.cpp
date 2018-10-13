//
// Created by 赵立伟 on 2018/9/21.
//

#include <cstring>
#include <iostream>
#include "TestDictType.h"

uint64_t TestDictType::hashFunction(const void *key) const {
    return 0;
}

void* TestDictType::keyDup(void *key) const {
}

void* TestDictType::valDup(void *obj) const {
}

int TestDictType::keyCompare(const void *key1, const void *key2) const {
    // return strcmp((const char*)key1, (const char*)key2);
    return -1;
}

void TestDictType::keyDestructor(void *key) const {
}

void TestDictType::valDestructor(void *obj) const {
}

void testScanProc(void* priv, void* key, void* val) {
    std::cout <<"key: " << (char*)key << " val: " << (char*)val << std::endl;
}
