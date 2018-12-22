//
// Created by 赵立伟 on 2018/9/18.
//

#ifndef FLYDB_FLYDB_H
#define FLYDB_FLYDB_H

#include <string>
#include "interface/AbstractFlyDB.h"
#include "../flyObj/interface/FlyObj.h"
#include "../coordinator/interface/AbstractCoordinator.h"

template<class KEY, class VAL>
class Dict;

class FlyDB : public AbstractFlyDB {
public:
    FlyDB(const AbstractCoordinator *coordinator);
    ~FlyDB();
    int expandDict(uint64_t size);
    int expandExpire(uint64_t size);
    void add(std::string *key, FlyObj *val);
    void addExpire(std::string *key, FlyObj *val, int64_t expire);
    void dictScan(
            Fio *fio,
            void (*scanProc)(void* priv, std::string *key, FlyObj *val));
    int64_t getExpire(std::string *key);
    const AbstractCoordinator* getCoordinator() const;
    uint32_t dictSize() const;
    uint32_t expireSize() const;

private:
    Dict<std::string, FlyObj>* dict;
    Dict<std::string, int64_t>* expires;
    const AbstractCoordinator *coordinator;
};


#endif //FLYDB_FLYDB_H
