//
// Created by 赵立伟 on 2018/10/13.
//

#include <string>
#include <list>
#include <mach/mach.h>
#include "FlyObj.h"
#include "../dict/Dict.h"
#include "../intset/IntSet.h"
#include "../skiplist/SkipList.h"

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
        switch (this->encoding) {
            case FLY_ENCODING_INT:
                delete reinterpret_cast<int *>(ptr);
                break;
            case FLY_ENCODING_STRING:
                delete reinterpret_cast<std::string *>(ptr);
                break;
            case FLY_ENCODING_HT:
                delete reinterpret_cast<Dict*>(ptr);
                break;
            case FLY_ENCODING_LINKEDLIST:
                delete reinterpret_cast<std::list<std::string> *>(ptr);
                break;
            case FLY_ENCODING_INTSET:
                delete reinterpret_cast<IntSet *>(ptr);
                break;
            case FLY_ENCODING_SKIPLIST:
                delete reinterpret_cast<SkipList *>(ptr);
                break;
            default:
                panic("wrong fly encoding!");
        }
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
