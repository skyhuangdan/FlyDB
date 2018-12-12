//
// Created by 赵立伟 on 2018/9/21.
//

#include "DictEntry.h"

template<class KEY, class VAL>
DictEntry<KEY, VAL>::DictEntry(KEY *key, VAL *val)
        : key(key), val(val) {}

template<class KEY, class VAL>
DictEntry<KEY, VAL>::~DictEntry() {
    delete this->key;
    delete this->val;
}

template<class KEY, class VAL>
KEY *DictEntry<KEY, VAL>::getKey() const {
    return this->key;
}

template<class KEY, class VAL>
VAL *DictEntry<KEY, VAL>::getVal() const {
    return this->val;
}

template<class KEY, class VAL>
void DictEntry<KEY, VAL>::setVal(void *val) {
    this->val = val;
}
