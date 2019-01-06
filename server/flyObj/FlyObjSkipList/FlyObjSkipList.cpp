//
// Created by 赵立伟 on 2018/12/9.
//

#include <string>
#include "FlyObjSkipList.h"
#include "../../dataStructure/skiplist/SkipList.cpp"

FlyObjSkipList::FlyObjSkipList(FlyObjType type) : FlyObj(type) {
    this->ptr = new SkipList<std::string>();
}

FlyObjSkipList::FlyObjSkipList(void *ptr, FlyObjType type) : FlyObj(ptr, type) {
}

FlyObjSkipList::~FlyObjSkipList() {
    delete this->ptr;
}
