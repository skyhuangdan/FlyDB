//
// Created by 赵立伟 on 2018/10/3.
//

#ifndef FLYDB_INTSET_H
#define FLYDB_INTSET_H

#include <cstdint>
#include "IntSetDef.h"

/**
 * 有序整数集合: 从小到大排列
 */
class IntSet {
public:
    IntSet();
    virtual ~IntSet();
    int add(int64_t value);
    int remove(int64_t value);
    bool find(int64_t value);
    int get(uint32_t index, int64_t* pValue);
    int32_t lenth();
    int64_t blobLength();
    int64_t random();
    int resize(uint32_t newLength);
    int upgradeAndAdd(int64_t value);
    void set(uint32_t pos, int64_t value);
    int search(int64_t value, uint32_t* pos);

private:
    int64_t getEncoded(uint32_t pos, uint8_t encoding);
    uint8_t valueEncoding(int64_t value);
    void moveTail(uint32_t from, uint32_t to);
    uint8_t encoding;
    uint32_t length;
    int8_t* contents;
};


#endif //FLYDB_INTSET_H
