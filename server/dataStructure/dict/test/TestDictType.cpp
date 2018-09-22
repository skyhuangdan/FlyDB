//
// Created by 赵立伟 on 2018/9/21.
//

#include <cstring>
#include <iostream>
#include "TestDictType.h"

unsigned long TestDictType::hashFunction(const void *key) {
    return 0;
}

void* TestDictType::keyDup(const void *key) {
}

void* TestDictType::valDup(const void *obj) {
}

int TestDictType::keyCompare(const void *key1, const void *key2) {
    // return strcmp((const char*)key1, (const char*)key2);
    return -1;
}

void TestDictType::keyDestructor(void *key) {
}

void TestDictType::valDestructor(void *obj) {
}

void testScanProc(void* priv, void* key, void* val) {
    std::cout <<"key: " << (char*)key << " val: " << (char*)val << std::endl;
}
