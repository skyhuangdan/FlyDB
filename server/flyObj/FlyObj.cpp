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
}

FlyObj::~FlyObj() {
    delete this->ptr;
}


FlyObjType FlyObj::getType() const {
    return this->type;
}

void FlyObj::setType(FlyObjType type) {
    this->type = type;
}

uint64_t FlyObj::getLru() const {
    return this->lru;
}

void FlyObj::setLru(uint64_t lru) {
    this->lru = lru;
}

void *FlyObj::getPtr() const {
    return this->ptr;
}

void FlyObj::setPtr(void *ptr) {
    this->ptr = ptr;
}
