//
// Created by 赵立伟 on 2018/12/3.
//

#ifndef FLYDB_ABSTRACTFLYDB_H
#define FLYDB_ABSTRACTFLYDB_H

#include <cstdint>

class AbstractFlyDB {
public:

    virtual int expandDict(uint64_t size) = 0;

    virtual int expandExpire(uint64_t size) = 0;
};

#endif //FLYDB_ABSTRACTFLYDB_H
