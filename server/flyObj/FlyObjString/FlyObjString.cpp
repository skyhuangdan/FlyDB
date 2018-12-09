//
// Created by 赵立伟 on 2018/12/9.
//

#include <string>
#include "FlyObjString.h"

FlyObjString::FlyObjString(FlyObjType type) : FlyObj(type) {
    this->ptr = new std::string;
}

FlyObjString::FlyObjString(void *ptr, FlyObjType type) : FlyObj(ptr, type) {
}

void FlyObjString::decrRefCount() {
    this->refCount--;
    if (0 == this->refCount) {
        delete reinterpret_cast<std::string *> (ptr);
    }
}

FlyObjEncode FlyObjString::getEncode() {
    return FLY_ENCODING_STRING;
}
