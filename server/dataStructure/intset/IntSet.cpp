//
// Created by 赵立伟 on 2018/10/3.
//

#include <cstdlib>
#include <cstring>
#include <ctime>
#include "IntSet.h"
#include "../../utils/EndianConvTool.h"

IntSet::IntSet() {
    this->encoding = INTSET_ENC_INT16;
    this->length = 0;
    this->contents = NULL;
    this->endianConvTool = EndianConvTool::getInstance();
}

IntSet::~IntSet() {
    free(this->contents);
}

int IntSet::add(int64_t value) {
    uint8_t encoding = valueEncoding(value);
    if (encoding > this->encoding) {
        upgradeAndAdd(value);
    } else {
        uint32_t pos = 0;
        if (search(value, &pos) > 0) {
            return 0;
        }

        // 扩容+1
        resize(this->length + 1);
        if (pos < this->length) {
            moveTail(pos, pos+1);
        }
        set(pos, value);
        this->length++;
    }
    return 1;
}

int IntSet::remove(int64_t value) {
    uint32_t pos = 0;
    if (this->valueEncoding(value) > this->encoding
        || search(value, &pos) <= 0) {
        return -1;
    }

    // 删除操作
    if (pos < this->length - 1) moveTail(pos+1, pos);
    resize(--this->length);

    return 1;
}

bool IntSet::find(int64_t value) {
    uint32_t pos = 0;
    if (search(value, &pos) > 0) {
        return true;
    } else {
        return false;
    }
}

int IntSet::get(uint32_t index, int64_t* pValue) {
    if (NULL == pValue || index >= this->length) {
        return -1;
    }

    *pValue = getEncoded(index, this->encoding);
    return 1;
}

int32_t IntSet::lenth() {
    return this->length;
}

int64_t IntSet::blobLength() {
    return this->length * this->encoding;
}

int64_t IntSet::random() {
    srand((unsigned)time(NULL));
    uint32_t index = rand() % this->length;

    int64_t value = 0;
    get(index, &value);
    return value;
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

void IntSet::moveTail(uint32_t from, uint32_t to) {
    void *dst, *src;
    if (INTSET_ENC_INT64 == this->encoding) {
        src = reinterpret_cast<int64_t*> (this->contents) + from;
        dst = reinterpret_cast<int64_t*> (this->contents) + to;
    } else if (INTSET_ENC_INT32 == this->encoding) {
        src = reinterpret_cast<int32_t*> (this->contents) + from;
        dst = reinterpret_cast<int32_t*> (this->contents) + to;
    } else {
        src = reinterpret_cast<int16_t*> (this->contents) + from;
        dst = reinterpret_cast<int16_t*> (this->contents) + to;
    }

    memmove(dst, src, (this->length - from) * this->encoding);
}

int IntSet::upgradeAndAdd(int64_t value) {
    int prepend = value < 0 ? 1 : 0;

    /// 先扩容
    uint8_t oldenc = this->encoding;
    this->encoding = valueEncoding(value);
    resize(this->length + 1);

    /// 重置contents中数据
    for (int i = this->length - 1; i >= 0; i--) {
        this->set(i + prepend, getEncoded(i, oldenc));
    }
    if (1 == prepend) {
        this->set(0, value);
    } else {
        this->set(this->length, value);
    }

    this->length++;
}

int IntSet::resize(uint32_t newLength) {
    uint32_t size = newLength * this->encoding;
    this->contents = reinterpret_cast<int8_t*>(realloc(this->contents, size));
}

int64_t IntSet::getEncoded(uint32_t pos, uint8_t encoding) {
    if (INTSET_ENC_INT64 == encoding) {
        int64_t res64;
        memcpy(&res64,
               reinterpret_cast<int64_t*>(this->contents) + pos,
               encoding);
        this->endianConvTool->memrev64ifbe(&res64);
        return res64;
    } else if (INTSET_ENC_INT32 == encoding) {
        int32_t res32;
        memcpy(&res32,
               reinterpret_cast<int32_t*>(this->contents) + pos,
               encoding);
        this->endianConvTool->memrev32ifbe(&res32);
        return res32;
    } else {
        int16_t res16;
        memcpy(&res16,
               reinterpret_cast<int16_t*>(this->contents) + pos,
               encoding);
        this->endianConvTool->memrev16ifbe(&res16);
        return res16;
    }
}

void IntSet::set(uint32_t pos, int64_t value) {
    if (INTSET_ENC_INT64 == this->encoding) {
        reinterpret_cast<int64_t*>(this->contents)[pos] = value;
        this->endianConvTool->memrev64ifbe(
                reinterpret_cast<int64_t*>(this->contents) + pos);
    } else if (INTSET_ENC_INT32 == this->encoding) {
        reinterpret_cast<int32_t*>(this->contents)[pos] = (int32_t)value;
        this->endianConvTool->memrev32ifbe(
                reinterpret_cast<int32_t*>(this->contents) + pos);
    } else {
        reinterpret_cast<int16_t*>(this->contents)[pos] = (int16_t)value;
        this->endianConvTool->memrev16ifbe(
                reinterpret_cast<int16_t*>(this->contents) + pos);
    }
}

int IntSet::search(int64_t value, uint32_t* pos) {
    // 入参异常
    if (NULL == pos) {
        return -1;
    }

    // 找不到
    if (0 == this->length) {
        *pos = 0;
        return 0;
    }

    // 折半查找
    int64_t min = 0, max = this->length - 1, mid;
    int64_t dstValue;
    while (max >= min) {
        mid = (max + min) / 2;
        get(mid, &dstValue);
        if (dstValue > value) {
            max = mid - 1;
        } else if (dstValue < value) {
            min = mid + 1;
        } else {
            *pos = mid;
            return 1;
        }
    }

    *pos = min;
    return 0;
}
