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
#include "FlyObjDeleter.h"

FlyObj::FlyObj(FlyObjType type) {
    this->type = type;
}

FlyObj::FlyObj(std::shared_ptr<char> ptr, FlyObjType type) {
    this->ptr = ptr;
    this->type = type;
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
    /**
     * 这里没有返回智能指针，因为该智能指针主要用于FlyObj浅拷贝时，
     * 增加智能指针的引用计数，防止有FlyObj释放时释放了ptr所占用空间
     **/
    return this->ptr.get();
}
