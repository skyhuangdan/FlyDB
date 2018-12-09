//
// Created by 赵立伟 on 2018/12/9.
//

#include "FlyObjIntFactory.h"
#include "FlyObjInt.h"

FlyObj* FlyObjIntFactory::getObject() {
    return new FlyObjInt(FLY_TYPE_STRING);
}

FlyObj* FlyObjIntFactory::getObject(void *ptr) {
    return new FlyObjInt(ptr, FLY_TYPE_STRING);
}
