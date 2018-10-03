//
// Created by 赵立伟 on 2018/10/3.
//

#include "IntSet.h"

IntSet::IntSet() {
    this->encoding = INTSET_ENC_INT16;
    this->length = 0;
}

int IntSet::add(int64_t value) {
    uint8_t encoding = valueEncoding(value);
    if (encoding > this->encoding) {
        upgradeAndAdd(value);
    } else {

    }

}

int IntSet::remove(int64_t value) {

}

bool IntSet::find(int64_t value) {

}

int64_t IntSet::get(uint32_t index) {

}

int32_t IntSet::lenth() {

}

int64_t IntSet::blobLength() {

}

int64_t IntSet::random() {

}

uint8_t IntSet::valueEncoding(int64_t value) {
    if (value < INT32_MIN || value > INT32_MAX) {
        return INTSET_ENC_INT64;
    } else if (value < INT16_MIN || value > INT16_MAX) {
        return INTSET_ENC_INT32;
    } else {
        return INTSET_ENC_INT16;
    }
}

int IntSet::upgradeAndAdd(int64_t value) {

}

