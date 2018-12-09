//
// Created by 赵立伟 on 2018/12/9.
//

#include "FlyObjLinkedListFactory.h"
#include "FlyObjLinkedList.h"

FlyObj* FlyObjLinkedListFactory::getObject() {
    return new FlyObjLinkedList(FLY_TYPE_LIST);
}

FlyObj* FlyObjLinkedListFactory::getObject(void *ptr) {
    return new FlyObjLinkedList(ptr, FLY_TYPE_LIST);
}
