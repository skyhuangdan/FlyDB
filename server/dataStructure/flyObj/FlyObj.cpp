//
// Created by 赵立伟 on 2018/10/13.
//

#include "FlyObj.h"

void FlyObj::incrRefCount() {
    this->refCount++;
}

void FlyObj::decrRefCount() {
    this->refCount--;
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

FlyObjEncoding FlyObj::getEncoding() const {
    return this->encoding;
}

void FlyObj::setEncoding(FlyObjEncoding encoding) {
    this->encoding = encoding;
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
