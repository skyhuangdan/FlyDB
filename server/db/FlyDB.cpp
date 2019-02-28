//
// Created by 赵立伟 on 2018/9/18.
//

#include "FlyDB.h"
#include "../dataStructure/dict/Dict.cpp"

FlyDB::FlyDB(const AbstractCoordinator *coordinator, uint8_t id) {
    this->dict = new Dict<std::string, std::shared_ptr<FlyObj>>();
    this->expires = new Dict<std::string, int64_t>();
    this->coordinator = coordinator;
    this->id = id;
}

FlyDB::~FlyDB() {
    delete this->dict;
    delete this->expires;
}

int FlyDB::expandDict(uint64_t size) {
    return this->dict->expand(size);
}

int FlyDB::expandExpire(uint64_t size) {
    return this->expires->expand(size);
}

int FlyDB::add(const std::string &key, std::shared_ptr<FlyObj> val) {
    return this->dict->addEntry(key, val);
}

int FlyDB::addExpire(const std::string &key,
                     std::shared_ptr<FlyObj> val,
                     int64_t expire) {
    if (-1 == this->add(key, val)) {
        return -1;
    }

    if (expire != -1) {
        this->expires->addEntry(key, expire);
    }

    return 1;
}

int FlyDB::dictScan(Fio *fio, scan scanProc) {
    uint32_t nextCur = 0;
    do {
        nextCur = this->dict->dictScan(
                nextCur,
                1,
                scanProc,
                new FioAndflyDB(fio, this));
        /** nextCur=-1代表遍历过程出错 */
        if (-1 == nextCur) {
            return -1;
        }
    } while (nextCur != 0);
}

int64_t FlyDB::getExpire(const std::string &key) {
    int64_t ex;
    int res = this->expires->fetchValue(key, &ex);
    if (-1 == res) {
        return -1;
    }
    return ex;
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

std::shared_ptr<FlyObj> FlyDB::lookupKey(const std::string &key) {
    DictEntry<std::string, std::shared_ptr<FlyObj>> *entry
            = this->dict->findEntry(key);
    if (NULL == entry) {
        return NULL;
    }

    int64_t expireTime = this->getExpire(key);
    if (expireTime != -1 && expireTime < time(NULL)) {
        this->delKey(key);
        /**
         * get delete command and
         * then feed to aof file(and rewrite block list)
         **/
        std::shared_ptr<FlyObj> *argvs = this->getDeleteCommandArgvs(key);
        coordinator->getAofHandler()->feedAppendOnlyFile(this->id, argvs, 2);
        return NULL;
    }

    std::shared_ptr<FlyObj> val = entry->getVal();
    val->setLru(miscTool->mstime());

    return val;
}

std::shared_ptr<FlyObj>* FlyDB::getDeleteCommandArgvs(
        const std::string &key) {
    std::shared_ptr<FlyObj> *argvs = new std::shared_ptr<FlyObj> [2];
    argvs[0] = coordinator->getFlyObjStringFactory()
            ->getObject(new std::string("del"));
    argvs[1] = coordinator->getFlyObjStringFactory()
            ->getObject(new std::string(key));

    return argvs;
}

void FlyDB::delKey(const std::string &key) {
    this->expires->deleteEntry(key);
    this->dict->deleteEntry(key);
}

int8_t FlyDB::getId() const {
    return this->id;
}
