//
// Created by 赵立伟 on 2018/12/9.
//

#include "FlyObjIntSet.h"
#include "../../dataStructure/intset/IntSet.h"
#include "../FlyObjDeleter.h"

FlyObjIntSet::FlyObjIntSet(FlyObjType type)
        : FlyObj(new IntSet(), type) {
}

FlyObjIntSet::FlyObjIntSet(void *ptr, FlyObjType type)
        : FlyObj(ptr, type) {

}

FlyObjIntSet::~FlyObjIntSet() {
    delete reinterpret_cast<IntSet *>(this->ptr);
}
