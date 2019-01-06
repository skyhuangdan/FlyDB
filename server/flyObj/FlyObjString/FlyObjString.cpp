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
