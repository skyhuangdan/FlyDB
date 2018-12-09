//
// Created by 赵立伟 on 2018/9/18.
//

#include "FlyDB.h"
#include "../dataStructure/dict/dictTypeImpl/DBDictType.h"
#include "../dataStructure/dict/dictTypeImpl/KeyPtrDictType.h"

FlyDB::FlyDB() {
    this->dict = new Dict(DBDictType::getInstance());
    this->expires = new Dict(KeyPtrDictType::getInstance());
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
