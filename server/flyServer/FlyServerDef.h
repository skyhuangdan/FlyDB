//
// Created by 赵立伟 on 2019/1/13.
//

#ifndef FLYDB_FLYSERVERDEF_H
#define FLYDB_FLYSERVERDEF_H

#include <cstdio>

struct PipeData {
    int processType;           /** AOF or RDB child? */
    size_t cowSize;            /** Copy on write size. */
    unsigned long long magic;   /** Magic value to make sure data is valid. */
};

#endif //FLYDB_FLYSERVERDEF_H
