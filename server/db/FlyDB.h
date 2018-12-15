//
// Created by 赵立伟 on 2018/9/18.
//

#ifndef FLYDB_FLYDB_H
#define FLYDB_FLYDB_H

#include <string>
#include "interface/AbstractFlyDB.h"
#include "../flyObj/interface/FlyObj.h"

template<class KEY, class VAL>
class Dict;

class FlyDB : public AbstractFlyDB {
public:
    FlyDB();
    ~FlyDB();
    int expandDict(uint64_t size);
    int expandExpire(uint64_t size);
    void add(std::string *key, FlyObj *val);
    void addExpire(std::string *key, FlyObj *val, uint64_t expire);

private:
    Dict<std::string, FlyObj>* dict;
    Dict<std::string, uint64_t>* expires;
};


#endif //FLYDB_FLYDB_H
