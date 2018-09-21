//
// Created by 赵立伟 on 2018/9/21.
//

#include "TestDictType.h"

unsigned int TestDictType::hashFunction(const void *key) {
    return 1;
}

void* TestDictType::keyDup(const void *key) {
}

void* TestDictType::valDup(const void *obj) {
}

int TestDictType::keyCompare(const void *key1, const void *key2) {
    return 1;
}

void TestDictType::keyDestructor(void *key) {
}

void TestDictType::valDestructor(void *obj) {

}
