//
// Created by 赵立伟 on 2018/12/9.
//

#include <string>
#include "SkipListStringType.h"

SkipListType* SkipListStringType::getInstance() {
    static SkipListStringType* instance;
    if (NULL == instance) {
        instance = new SkipListStringType();
    }
    return instance;
}

int SkipListStringType::compare(const void* val1, const void* val2) const {
    const std::string *str1 = reinterpret_cast<const std::string *>(val1);
    const std::string *str2 = reinterpret_cast<const std::string *>(val2);

    return strcmp(str1->c_str(), str2->c_str());
}

void* SkipListStringType::dup(const void* val) const {
    std::string *res = new std::string;
    res->assign(*(reinterpret_cast<const std::string *>(val)));
    return res;

}

void SkipListStringType::destructor(void *obj) const {
    delete reinterpret_cast<const std::string *>(obj);
}
