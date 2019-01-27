//
// Created by 赵立伟 on 2018/12/9.
//

#include <string>
#include "FlyObjString.h"
#include "../FlyObjDeleter.h"

FlyObjString::FlyObjString(FlyObjType type) : FlyObj(
        std::shared_ptr<char>((char*)new std::string,
                              MyDeleter<std::string>()), type) {
}

FlyObjString::FlyObjString(void *ptr, FlyObjType type) : FlyObj(
        std::shared_ptr<char>((char*)ptr, MyDeleter<std::string>()), type) {

}
