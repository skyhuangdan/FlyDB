//
// Created by 赵立伟 on 2018/9/18.
//

#ifndef FLYDB_FLYDB_H
#define FLYDB_FLYDB_H

#include "../dataStructure/dict/Dict.h"
#include "interface/AbstractFlyDB.h"

class FlyDB : public AbstractFlyDB {
public:
    FlyDB();
    int expandDict(uint64_t size);
    int expandExpire(uint64_t size);

private:
    Dict* dict;
    Dict* expires;
};


#endif //FLYDB_FLYDB_H
