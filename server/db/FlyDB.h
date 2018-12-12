//
// Created by 赵立伟 on 2018/9/18.
//

#ifndef FLYDB_FLYDB_H
#define FLYDB_FLYDB_H

#include <string>
#include "interface/AbstractFlyDB.h"

template<class KEY, class VAL>
class Dict;

class FlyDB : public AbstractFlyDB {
public:
    FlyDB();
    ~FlyDB();
    int expandDict(uint64_t size);
    int expandExpire(uint64_t size);

private:
    Dict<std::string, void>* dict;
    Dict<std::string, void>* expires;
};


#endif //FLYDB_FLYDB_H
