//
// Created by 赵立伟 on 2018/12/9.
//

#include "FlyObjHashTable.h"
#include "../../dataStructure/dict/Dict.h"
#include "../FlyObjDeleter.h"

FlyObjHashTable::FlyObjHashTable(FlyObjType type) : FlyObj(
        std::shared_ptr<char>((char*)new Dict<std::string, std::string>(),
                MyDeleter<Dict<std::string, std::string>>()), type) {
}

FlyObjHashTable::FlyObjHashTable(void *ptr, FlyObjType type) : FlyObj(
        std::shared_ptr<char>((char*)ptr,
                MyDeleter<Dict<std::string, std::string>>()), type) {
}
