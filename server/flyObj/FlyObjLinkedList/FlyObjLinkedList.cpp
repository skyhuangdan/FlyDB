//
// Created by 赵立伟 on 2018/12/9.
//

#include <string>
#include <list>
#include "FlyObjLinkedList.h"

FlyObjLinkedList::FlyObjLinkedList(FlyObjType type) : FlyObj(type) {
    this->ptr = new std::list<std::string>;
}

FlyObjLinkedList::FlyObjLinkedList(void *ptr, FlyObjType type) : FlyObj(ptr, type) {
}

void FlyObjLinkedList::decrRefCount() {
    this->refCount--;
    if (0 == this->refCount) {
        delete reinterpret_cast<std::list<std::string> *> (ptr);
    }
}

FlyObjEncode FlyObjLinkedList::getEncode() {
    return FLY_ENCODING_LINKEDLIST;
}
