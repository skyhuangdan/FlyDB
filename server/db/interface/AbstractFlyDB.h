//
// Created by 赵立伟 on 2018/12/3.
//

#ifndef FLYDB_ABSTRACTFLYDB_H
#define FLYDB_ABSTRACTFLYDB_H

#include <cstdint>
#include <string>

class FlyObj;

class AbstractFlyDB {
public:

    virtual int expandDict(uint64_t size) = 0;

    virtual int expandExpire(uint64_t size) = 0;

    virtual void add(std::string *key, FlyObj *val) = 0;

    virtual void addExpire(std::string *key,
                           FlyObj *val,
                           uint64_t expire) = 0;
};

#endif //FLYDB_ABSTRACTFLYDB_H
