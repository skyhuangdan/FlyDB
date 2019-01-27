//
// Created by 赵立伟 on 2018/12/9.
//

#include "FlyObjIntSet.h"
#include "../../dataStructure/intset/IntSet.h"
#include "../FlyObjDeleter.h"

FlyObjIntSet::FlyObjIntSet(FlyObjType type) : FlyObj(
                std::shared_ptr<char>((char*)new IntSet(), MyDeleter<IntSet>()),
                type) {
}

FlyObjIntSet::FlyObjIntSet(void *ptr, FlyObjType type) : FlyObj(
        std::shared_ptr<char>((char*)ptr, MyDeleter<IntSet>()),
        type) {

}
