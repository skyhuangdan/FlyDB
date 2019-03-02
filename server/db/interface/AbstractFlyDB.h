//
// Created by 赵立伟 on 2018/12/3.
//

#ifndef FLYDB_ABSTRACTFLYDB_H
#define FLYDB_ABSTRACTFLYDB_H

#include <cstdint>
#include <string>
#include "../../io/base/Fio.h"

class FlyObj;
class AbstractFlyDB;
class AbstractCoordinator;

struct FioAndflyDB {
    FioAndflyDB(Fio *fio, AbstractFlyDB *flyDB) {
        this->fio = fio;
        this->flyDB = flyDB;
    }

    Fio *fio;
    AbstractFlyDB *flyDB;
};

struct FioAndCoord {
    FioAndCoord(Fio *fio, const AbstractCoordinator *coord) {
        this->fio = fio;
        this->coord = coord;
    }

    Fio *fio;
    const AbstractCoordinator *coord;
};

typedef int (*scan)(void* priv, std::string key, std::shared_ptr<FlyObj> val);

class AbstractFlyDB {
public:
    virtual ~AbstractFlyDB() {};

    virtual int expandDict(uint64_t size) = 0;

    virtual int expandExpire(uint64_t size) = 0;

    virtual int add(const std::string &key, std::shared_ptr<FlyObj> val) = 0;

    virtual int addExpire(const std::string &key,
                          std::shared_ptr<FlyObj> val,
                          uint64_t expire) = 0;

    virtual int dictScan(Fio *fio, scan scanProc) = 0;

    virtual uint64_t getExpire(const std::string &key) = 0;

    virtual const AbstractCoordinator* getCoordinator() const = 0;

    virtual uint32_t dictSlotNum() const = 0;

    virtual uint32_t expireSlotNum() const = 0;

    virtual uint32_t dictSize() const = 0;

    virtual uint32_t expireSize() const = 0;

    virtual std::shared_ptr<FlyObj> lookupKey(const std::string &key) = 0;

    virtual void deleteKey(const std::string &key) = 0;

    virtual int8_t getId() const = 0;

    virtual bool activeExpireCycle(uint64_t start,
                                   uint64_t timelimit) = 0;

    virtual void tryResizeDB() = 0;
};

#endif //FLYDB_ABSTRACTFLYDB_H
