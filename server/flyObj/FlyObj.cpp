//
// Created by 赵立伟 on 2018/10/13.
//

#include <string>
#include <list>
#include <mach/mach.h>
#include "FlyObj.h"
#include "../dataStructure/dict/Dict.h"
#include "../dataStructure/intset/IntSet.h"
#include "../dataStructure/skiplist/SkipList.h"
#include "FlyObjDef.h"

FlyObj::FlyObj(void *ptr, FlyObjType type) {
    this->ptr = ptr;
    this->type = type;
    this->refCount = 1;
    // todo encoding / lru
}

void FlyObj::incrRefCount() {
    this->refCount++;
}

void FlyObj::decrRefCount() {
    this->refCount--;
    if (0 == this->refCount) {
        this->encoding->destructor(this->ptr);
    }
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

const FlyObjEncoding *FlyObj::getEncoding() const {
    return this->encoding;
}

void FlyObj::setEncoding(const FlyObjEncoding *encoding) {
    this->encoding = encoding;
}

