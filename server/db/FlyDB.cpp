//
// Created by 赵立伟 on 2018/9/18.
//

#include "FlyDB.h"
#include "../dataStructure/dict/Dict.cpp"

FlyDB::FlyDB() {
    this->dict = new Dict<std::string, FlyObj>();
    this->expires = new Dict<std::string, int64_t>();
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
        void (*scanProc)(void* priv, std::string *key, FlyObj *val)) {
    uint32_t nextCur = 0;
    do {
        nextCur = this->dict->dictScan(nextCur, 1,  scanProc, NULL);
    } while (nextCur != 0);
}

int64_t FlyDB::getExpire(std::string *key) {
    return *(this->expires->fetchValue(key));
}
