//
// Created by 赵立伟 on 2018/12/9.
//

#include "FlyObjHashTableFactory.h"
#include "FlyObjHashTable.h"

FlyObjHashTableFactory::~FlyObjHashTableFactory() {

}

FlyObj* FlyObjHashTableFactory::getObject() {
    return new FlyObjHashTable(FLY_TYPE_HASH);
}

FlyObj* FlyObjHashTableFactory::getObject(void *ptr) {
    return new FlyObjHashTable(ptr, FLY_TYPE_HASH);
}
