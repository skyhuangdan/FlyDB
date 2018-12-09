//
// Created by 赵立伟 on 2018/12/9.
//

#include "FlyObjIntSetFactory.h"
#include "FlyObjIntSet.h"

FlyObj* FlyObjIntSetFactory::getObject() {
    return new FlyObjIntSet(FLY_TYPE_SET);
}

FlyObj* FlyObjIntSetFactory::getObject(void *ptr) {
    return new FlyObjIntSet(ptr, FLY_TYPE_SET);
}
