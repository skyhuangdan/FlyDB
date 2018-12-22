//
// Created by 赵立伟 on 2018/12/3.
//

#ifndef FLYDB_ABSTRACTFLYDB_H
#define FLYDB_ABSTRACTFLYDB_H

#include <cstdint>
#include <string>
#include "../../io/interface/Fio.h"

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

class AbstractFlyDB {
public:

    virtual int expandDict(uint64_t size) = 0;

    virtual int expandExpire(uint64_t size) = 0;

    virtual void add(std::string *key, FlyObj *val) = 0;

    virtual void addExpire(std::string *key,
                           FlyObj *val,
                           int64_t expire) = 0;

    virtual void dictScan(
            Fio *fio,
            void (*scanProc)(void* priv, std::string *key, FlyObj *val)) = 0;

    virtual int64_t getExpire(std::string *key) = 0;

    virtual const AbstractCoordinator* getCoordinator() const = 0;

    virtual uint32_t dictSize() const = 0;

    virtual uint32_t expireSize() const = 0;
};

#endif //FLYDB_ABSTRACTFLYDB_H
