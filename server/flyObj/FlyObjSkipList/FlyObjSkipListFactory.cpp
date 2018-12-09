//
// Created by 赵立伟 on 2018/12/9.
//

#include "FlyObjSkipListFactory.h"
#include "FlyObjSkipList.h"

FlyObj* FlyObjSkipListFactory::getObject() {
    return new FlyObjSkipList(FLY_TYPE_LIST);
}

FlyObj* FlyObjSkipListFactory::getObject(void *ptr) {
    return new FlyObjSkipList(ptr, FLY_TYPE_LIST);
}
