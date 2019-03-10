//
// Created by 赵立伟 on 2018/9/18.
//

#include "FlyDB.h"
#include "../dataStructure/dict/Dict.cpp"
#include "FlyDBDef.h"

FlyDB::FlyDB(const AbstractCoordinator *coordinator, uint8_t id) {
    this->dict = new Dict<std::string, std::shared_ptr<FlyObj>>();
    this->expires = new Dict<std::string, uint64_t>();
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
                     uint64_t expire) {
    if (-1 == this->add(key, val)) {
        return -1;
    }

    if (expire != -1) {
        this->expires->addEntry(key, expire);
    }

    return 1;
}

int FlyDB::dictScan(std::shared_ptr<Fio> fio, scan scanProc) {
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

uint64_t FlyDB::getExpire(const std::string &key) {
    uint64_t ex;
    int res = this->expires->fetchValue(key, &ex);
    if (-1 == res) {
        return -1;
    }
    return ex;
}

const AbstractCoordinator* FlyDB::getCoordinator() const {
    return this->coordinator;
}

uint32_t FlyDB::dictSlotNum() const {
    return dict->slotNum();
}

uint32_t FlyDB::expireSlotNum() const {
    return expires->slotNum();
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

    uint64_t expireTime = this->getExpire(key);
    if (expireTime != -1 && expireTime < time(NULL)) {
        this->deleteKey(key);
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

void FlyDB::deleteKey(const std::string &key) {
    this->expires->deleteEntry(key);
    this->dict->deleteEntry(key);

    /**
     * get delete command and
     * then feed to aof file(and rewrite block list)
     **/
    std::shared_ptr<FlyObj> *argvs = this->getDeleteCommandArgvs(key);
    coordinator->getAofHandler()->feedAppendOnlyFile(this->id, argvs, 2);
}

int8_t FlyDB::getId() const {
    return this->id;
}

bool FlyDB::activeExpireCycle(uint64_t start, uint64_t timelimit) {
    int expired = 0;
    do {
        uint32_t num = 0;
        /** 如果当前db为空，则开始执行下一个 */
        if (0 == (num = this->dictSize())) {
            continue;
        }

        /** 如果当前db占用比例小于1%，则不处理 */
        uint32_t slots = this->dictSlotNum();
        if (num * 100 / slots < 1) {
            break;
        }

        /** 每次loop最多只处理ACTIVE_EXPIRE_CYCLE_LOOKUPS_PER_LOOP个过期键 */
        if (num > ACTIVE_EXPIRE_CYCLE_LOOKUPS_PER_LOOP) {
            num = ACTIVE_EXPIRE_CYCLE_LOOKUPS_PER_LOOP;
        }

        while (num--) {
            DictEntry<std::string, uint64_t>* expireEntry =
                    this->expires->getRandomEntry();
            uint64_t expiretime = expireEntry->getVal();
            if (expireEntry->getVal() <= start) {
                this->deleteKey(expireEntry->getKey());
                expired++;
            }
        }

        /** 如果超过时间限制，则返回 */
        if (miscTool->ustime() - start > timelimit) {
            return true;
        }

        /**
         * 如果该次循环删除的key数量
         * 小于ACTIVE_EXPIRE_CYCLE_LOOKUPS_PER_LOOP的1/4，则扫描下一个db
         **/
    } while (expired > ACTIVE_EXPIRE_CYCLE_LOOKUPS_PER_LOOP / 4);

    return false;
}

void FlyDB::tryResizeDB() {
    this->dict->tryShrink();
    this->expires->tryShrink();
}
