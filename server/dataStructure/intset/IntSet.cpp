//
// Created by 赵立伟 on 2018/10/3.
//

#include <cstdlib>
#include <cstring>
#include "IntSet.h"

IntSet::IntSet() {
    this->encoding = INTSET_ENC_INT16;
    this->length = 0;
    this->contents = NULL;
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
    bool prepend = value < 0 ? 1 : 0;

    /// 先扩容
    uint8_t oldenc = this->encoding;
    this->encoding = valueEncoding(value);
    resize(this->length + 1);

    // todo : set
}

int IntSet::resize(uint32_t newLength) {
    uint32_t size = this->length * this->encoding;
    this->contents = (int8_t* )realloc(this->contents, size);
}

int64_t IntSet::getEncoded(uint32_t pos, uint8_t encoding) {
    if (INTSET_ENC_INT64 == encoding) {
        int64_t res64;
        memcpy(&res64, ((int64_t*)this->contents) + pos, encoding);
        return res64;
    } else if (INTSET_ENC_INT32 == encoding) {
        int32_t res32;
        memcpy(&res32, ((int32_t*)this->contents) + pos, encoding);
        return res32;
    } else {
        int16_t res16;
        memcpy(&res16, ((int16_t*)this->contents) + pos, encoding);
        return res16;
    }
}

void IntSet::set(uint32_t pos, int64_t value) {
    if (INTSET_ENC_INT64 == this->encoding) {

    } else if (INTSET_ENC_INT32 == this->encoding) {

    } else {

    }
}
