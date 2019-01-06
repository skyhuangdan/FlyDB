//
// Created by 赵立伟 on 2018/12/9.
//

#include "FlyObjInt.h"

FlyObjInt::FlyObjInt(FlyObjType type) : FlyObj(type) {
    this->ptr = new int;
}

FlyObjInt::FlyObjInt(void *ptr, FlyObjType type) : FlyObj(ptr, type) {

}
