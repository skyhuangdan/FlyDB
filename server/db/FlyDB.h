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

template<class KEY, class VAL>
class DictEntry;

class FlyDB : public AbstractFlyDB {
public:
    FlyDB(const AbstractCoordinator *coordinator, uint8_t id);
    ~FlyDB();
    int expandDict(uint64_t size);
    int expandExpire(uint64_t size);
    int add(const std::string &key, std::shared_ptr<FlyObj> val);
    int addExpire(const std::string &key,
                  std::shared_ptr<FlyObj> val, 
                  uint64_t expire);
    int dictScan(Fio *fio, scan scanProc);
    uint64_t getExpire(const std::string &key);
    const AbstractCoordinator* getCoordinator() const;
    uint32_t dictSlotNum() const;
    uint32_t expireSlotNum() const;
    uint32_t dictSize() const;
    uint32_t expireSize() const;
    std::shared_ptr<FlyObj> lookupKey(const std::string &key);
    void deleteKey(const std::string &key);
    int8_t getId() const;
    bool activeExpireCycle(int type, uint64_t start, uint64_t timelimit);

private:
    std::shared_ptr<FlyObj>* getDeleteCommandArgvs(const std::string &key);

    uint8_t id = 0;
    Dict<std::string, std::shared_ptr<FlyObj>>* dict;
    Dict<std::string, uint64_t>* expires;

    const AbstractCoordinator *coordinator;
};


#endif //FLYDB_FLYDB_H
