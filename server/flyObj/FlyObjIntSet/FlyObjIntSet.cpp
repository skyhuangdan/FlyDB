//
// Created by 赵立伟 on 2018/12/9.
//

#include "FlyObjIntSet.h"
#include "../../dataStructure/intset/IntSet.h"

FlyObjIntSet::FlyObjIntSet(FlyObjType type) : FlyObj(type) {
    this->ptr = new IntSet();
}

FlyObjIntSet::FlyObjIntSet(void *ptr, FlyObjType type) : FlyObj(ptr, type) {

}

FlyObjEncode FlyObjIntSet::getEncode() {
    return FLY_ENCODING_INTSET;
}
