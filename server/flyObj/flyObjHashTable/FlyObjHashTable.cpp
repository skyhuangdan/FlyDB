//
// Created by 赵立伟 on 2018/12/9.
//

#include "FlyObjHashTable.h"
#include "../../dataStructure/dict/Dict.h"

FlyObjHashTable::FlyObjHashTable(FlyObjType type) : FlyObj(type) {
    this->ptr = new Dict<std::string, std::string>();
}

FlyObjHashTable::FlyObjHashTable(void *ptr, FlyObjType type) : FlyObj(ptr, type) {
}
