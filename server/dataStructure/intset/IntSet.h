//
// Created by 赵立伟 on 2018/10/3.
//

#ifndef FLYDB_INTSET_H
#define FLYDB_INTSET_H

#include <cstdint>
#include "IntSetDef.h"

class IntSet {
public:
    IntSet();
    int add(int64_t value);
    int remove(int64_t value);
    bool find(int64_t value);
    int64_t get(uint32_t index);
    int32_t lenth();
    int64_t blobLength();
    int64_t random();

private:
    int upgradeAndAdd(int64_t value);
    uint8_t valueEncoding(int64_t value);
    uint8_t encoding;
    uint32_t length;
    int8_t contents[];
};


#endif //FLYDB_INTSET_H
