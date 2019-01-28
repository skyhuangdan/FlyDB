//
// Created by 赵立伟 on 2018/12/9.
//

#include "FlyObjIntSetFactory.h"
#include "FlyObjIntSet.h"

FlyObjIntSetFactory::~FlyObjIntSetFactory() {

}

std::shared_ptr<FlyObj> FlyObjIntSetFactory::getObject() {
    return std::shared_ptr<FlyObj>(new FlyObjIntSet(FLY_TYPE_SET));
}

std::shared_ptr<FlyObj> FlyObjIntSetFactory::getObject(void *ptr) {
    return std::shared_ptr<FlyObj>(new FlyObjIntSet(ptr, FLY_TYPE_SET));
}
