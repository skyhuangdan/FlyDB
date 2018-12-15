//
// Created by 赵立伟 on 2018/12/9.
//

#include <string>
#include <list>
#include "FlyObjLinkedList.h"
#include "../../dataStructure/skiplist/SkipList.h"

FlyObjLinkedList::FlyObjLinkedList(FlyObjType type) : FlyObj(type) {
    this->ptr = new SkipList<std::string>();
}

FlyObjLinkedList::FlyObjLinkedList(void *ptr, FlyObjType type) : FlyObj(ptr, type) {
}

FlyObjEncode FlyObjLinkedList::getEncode() {
    return FLY_ENCODING_LINKEDLIST;
}
