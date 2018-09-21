//
// Created by 赵立伟 on 2018/9/21.
//

#include "DictType.h"
#include "DictEntry.h"

DictEntry::DictEntry(void *key, void *val, DictType* const type) : key(key), val(val), type(type) {}

DictEntry::~DictEntry() {
    type->keyDestructor(this->key);
    type->valDestructor(this->val);
}

void *DictEntry::getKey() const {
    return key;
}

void *DictEntry::getVal() const {
    return val;
}

void DictEntry::setVal(void *val) {
    DictEntry::val = val;
}
