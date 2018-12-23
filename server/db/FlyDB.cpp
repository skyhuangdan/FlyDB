//
// Created by 赵立伟 on 2018/9/18.
//

#include "FlyDB.h"
#include "../dataStructure/dict/Dict.cpp"

FlyDB::FlyDB(const AbstractCoordinator *coordinator) {
    this->dict = new Dict<std::string, FlyObj>();
    this->expires = new Dict<std::string, int64_t>();
    this->coordinator = coordinator;
}

FlyDB::~FlyDB() {
    delete this->dict;
    delete this->expires;
}

int FlyDB::expandDict(uint64_t size) {
    this->dict->expand(size);
}

int FlyDB::expandExpire(uint64_t size) {
    this->expires->expand(size);
}

void FlyDB::add(std::string *key, FlyObj *val) {
    this->dict->addEntry(key, val);
}

void FlyDB::addExpire(std::string *key, FlyObj *val, int64_t expire) {
    this->add(key, val);
    if (expire != -1) {
        this->expires->addEntry(key, new int64_t(expire));
    }
}

void FlyDB::dictScan(
        Fio *fio,
        void (*scanProc)(void* priv, std::string *key, FlyObj *val)) {
    uint32_t nextCur = 0;
    do {
        nextCur = this->dict->dictScan(
                nextCur,
                1,
                scanProc,
                new FioAndflyDB(fio, this));
    } while (nextCur != 0);
}

int64_t FlyDB::getExpire(std::string *key) {
    return *(this->expires->fetchValue(key));
}

const AbstractCoordinator* FlyDB::getCoordinator() const {
    return this->coordinator;
}

uint32_t FlyDB::dictSize() const {
    return dict->size();
}

uint32_t FlyDB::expireSize() const {
    return expires->size();
}

FlyObj* FlyDB::lookupKey(std::string *key) {
    DictEntry<std::string, FlyObj> *entry = this->dict->findEntry(key);
    if (NULL == entry) {
        return NULL;
    }

    FlyObj *obj = entry->getVal();
    obj->setLru(miscTool->mstime());

    return obj;
}
