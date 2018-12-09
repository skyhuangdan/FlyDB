//
// Created by 赵立伟 on 2018/10/13.
//

#include <string>
#include <list>
#include <mach/mach.h>
#include "interface/FlyObj.h"
#include "../dataStructure/dict/Dict.h"
#include "../dataStructure/intset/IntSet.h"
#include "../dataStructure/skiplist/SkipList.h"
#include "FlyObjDef.h"

FlyObj::FlyObj(FlyObjType type) {
    this->type = type;
}

FlyObj::FlyObj(void *ptr, FlyObjType type) {
    this->ptr = ptr;
    this->type = type;
    this->refCount = 1;
}

void FlyObj::incrRefCount() {
    this->refCount++;
}

void FlyObj::resetRefCount() {
    this->refCount = 0;
}

FlyObjType FlyObj::getType() const {
    return this->type;
}

void FlyObj::setType(FlyObjType type) {
    this->type = type;
}

uint32_t FlyObj::getRefCount() const {
    return this->refCount;
}

void FlyObj::setRefCount(uint32_t refCount) {
    this->refCount = refCount;
}

uint16_t FlyObj::getLru() const {
    return this->lru;
}

void FlyObj::setLru(uint16_t lru) {
    this->lru = lru;
}

void *FlyObj::getPtr() const {
    return this->ptr;
}

void FlyObj::setPtr(void *ptr) {
    this->ptr = ptr;
}
