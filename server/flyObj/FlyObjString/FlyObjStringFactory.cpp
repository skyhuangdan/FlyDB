//
// Created by 赵立伟 on 2018/12/9.
//

#include "FlyObjStringFactory.h"
#include "FlyObjString.h"

FlyObj* FlyObjStringFactory::getObject(void *ptr) {
    return new FlyObjString(ptr, FLY_TYPE_STRING);
}

FlyObj* FlyObjStringFactory::getObject() {
    return new FlyObjString(FLY_TYPE_STRING);
}
