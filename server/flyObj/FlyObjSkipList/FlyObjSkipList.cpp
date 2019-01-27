//
// Created by 赵立伟 on 2018/12/9.
//

#include <string>
#include "FlyObjSkipList.h"
#include "../../dataStructure/skiplist/SkipList.cpp"
#include "../FlyObjDeleter.h"

FlyObjSkipList::FlyObjSkipList(FlyObjType type) : FlyObj(
        std::shared_ptr<char>((char*)new SkipList<std::string>,
                MyDeleter<SkipList<std::string>>()), type) {
}

FlyObjSkipList::FlyObjSkipList(void *ptr, FlyObjType type) : FlyObj(
        std::shared_ptr<char>((char*)ptr,
                MyDeleter<SkipList<std::string>>()), type) {
}
