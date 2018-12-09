//
// Created by 赵立伟 on 2018/12/9.
//

#include "FlyObjSkipList.h"
#include "../../dataStructure/skiplist/SkipList.h"
#include "../../dataStructure/skiplist/SkipListStringType.h"

FlyObjSkipList::FlyObjSkipList(FlyObjType type) : FlyObj(type) {
    this->ptr = new SkipList(SkipListStringType());
}

FlyObjSkipList::FlyObjSkipList(void *ptr, FlyObjType type) : FlyObj(ptr, type) {
}

void FlyObjSkipList::decrRefCount() {
    this->refCount--;
    if (0 == this->refCount) {
        delete reinterpret_cast<SkipList *> (ptr);
    }
}

FlyObjEncode FlyObjSkipList::getEncode() {
    return FLY_ENCODING_SKIPLIST;
}
