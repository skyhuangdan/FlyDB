//
// Created by 赵立伟 on 2018/9/21.
//

#include "DictEntry.h"

DictEntry::DictEntry(void *key, void *val, DictType* const type) : key(key), val(val), type(type) {}

DictEntry::~DictEntry() {
    type->keyDestructor(this->key);
    type->valDestructor(this->val);
}

void *DictEntry::getKey() const {
    return this->key;
}

void *DictEntry::getVal() const {
    return this->val;
}

void DictEntry::setVal(void *val) {
    this->val = type->valDup(val);
}
