//
// Created by 赵立伟 on 2018/9/18.
//

#ifndef FLYDB_FLYDB_H
#define FLYDB_FLYDB_H

#include <string>
#include "interface/AbstractFlyDB.h"
#include "../flyObj/FlyObj.h"
#include "../coordinator/interface/AbstractCoordinator.h"

template<class KEY, class VAL>
class Dict;

class FlyDB : public AbstractFlyDB {
public:
    FlyDB(const AbstractCoordinator *coordinator, uint8_t id);
    ~FlyDB();
    int expandDict(uint64_t size);
    int expandExpire(uint64_t size);
    int add(const std::string &key, std::shared_ptr<FlyObj> val);
    int addExpire(const std::string &key, 
                  std::shared_ptr<FlyObj> val, 
                  int64_t expire);
    int dictScan(Fio *fio, scan scanProc);
    int64_t getExpire(const std::string &key);
    const AbstractCoordinator* getCoordinator() const;
    uint32_t dictSize() const;
    uint32_t expireSize() const;
    std::shared_ptr<FlyObj> lookupKey(const std::string &key);
    void delKey(const std::string &key);
    int8_t getId() const;
    void activeExpireCycle(int type);

private:
    std::shared_ptr<FlyObj>* getDeleteCommandArgvs(const std::string &key);

    uint8_t id = 0;
    Dict<std::string, std::shared_ptr<FlyObj>>* dict;
    Dict<std::string, int64_t>* expires;

    const AbstractCoordinator *coordinator;
};


#endif //FLYDB_FLYDB_H
