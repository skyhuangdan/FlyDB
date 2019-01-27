//
// Created by 赵立伟 on 2018/12/9.
//

#include <string>
#include <list>
#include "FlyObjLinkedList.h"
#include "../../dataStructure/skiplist/SkipList.h"
#include "../FlyObjDeleter.h"

FlyObjLinkedList::FlyObjLinkedList(FlyObjType type) : FlyObj(
        std::shared_ptr<char>((char*)new std::list<std::string>,
                MyDeleter<std::list<std::string>>()), type) {
}

FlyObjLinkedList::FlyObjLinkedList(void *ptr, FlyObjType type) : FlyObj(
        std::shared_ptr<char>((char*)ptr,
                MyDeleter<std::list<std::string>>()), type) {
}
