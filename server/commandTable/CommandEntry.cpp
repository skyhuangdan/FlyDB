//
// Created by 赵立伟 on 2018/9/19.
//

#include <iostream>
#include "CommandEntry.h"
#include "../flyServer/interface/AbstractFlyServer.h"
#include "../dataStructure/skiplist/SkipList.cpp"
#include "../dataStructure/dict/Dict.cpp"
#include "CommandTableDef.h"

std::vector<CommandEntry* > flyDBCommandTable = {
       new CommandEntry("version",     versionCommand,     1, "rF",  0, NULL, 1, 1, 1, 0, 0),
       new CommandEntry("get",         getCommand,         2, "rF",  0, NULL, 1, 1, 1, 0, 0),
       new CommandEntry("set",         setCommand,        -3, "wm",  0, NULL, 1, 1, 1, 0, 0),
       new CommandEntry("expire",      expireCommand,      3, "wF",  0, NULL, 1, 1, 1, 0, 0),
       new CommandEntry("expireat",    expireatCommand,    3, "wF",  0, NULL, 1, 1, 1, 0, 0),
       new CommandEntry("mget",        mgetCommand,       -2, "rF",  0, NULL, 1,-1, 1, 0, 0),
       new CommandEntry("rpush",       rpushCommand,      -3, "wmF", 0, NULL, 1, 1, 1, 0, 0),
       new CommandEntry("lpush",       lpushCommand,      -3, "wmF", 0, NULL, 1, 1, 1, 0, 0),
       new CommandEntry("sortpush",    pushSortCommand,   -3, "wmF", 0, NULL, 1, 1, 1, 0, 0),
       new CommandEntry("rpop",        rpopCommand,        2, "wF",  0, NULL, 1, 1, 1, 0, 0),
       new CommandEntry("lpop",        lpopCommand,        2, "wF",  0, NULL, 1, 1, 1, 0, 0),
       new CommandEntry("sortPop",     popSortCommand,     2, "wF",  0, NULL, 1, 1, 1, 0, 0),
       new CommandEntry("hset",        hsetCommand,       -4, "wmF", 0, NULL, 1, 1, 1, 0, 0),
       new CommandEntry("hget",        hgetCommand,        3, "rF",  0, NULL, 1, 1, 1, 0, 0),
       new CommandEntry("save",        saveCommand,        1, "as",  0, NULL, 0, 0, 0, 0, 0),
       new CommandEntry("bgsave",      bgsaveCommand,     -1, "a",   0, NULL, 0, 0, 0, 0, 0),
       new CommandEntry("config",      configCommand,     -2, "lat", 0, NULL, 0, 0, 0, 0, 0),
       new CommandEntry("bgrewriteaof",bgrewriteaofCommand,1, "a",   0, NULL, 0, 0, 0, 0, 0),
       new CommandEntry("select",      selectCommand,      2, "lF", 0, NULL, 0, 0, 0, 0, 0)
};


CommandEntry::CommandEntry() {

}

CommandEntry::CommandEntry(commandProc proc, int flag) {
    this->proc = proc;
    this->flag = flag;
}

CommandEntry::CommandEntry(const char *name,
                           commandProc proc,
                           int arity,
                           const std::string &sflags,
                           int flag,
                           getKeysProc keysProc,
                           int firstKey,
                           int lastKey,
                           int keyStep,
                           uint64_t microseconds,
                           uint64_t calls) {
    this->name = name;
    this->proc = proc;
    this->arity = arity;
    this->sflags = sflags;
    this->flag = flag;
    this->keysProc = keysProc;
    this->firstKey = firstKey;
    this->lastKey = lastKey;
    this->keyStep = keyStep;
    this->microseconds = microseconds;
    this->calls = calls;
}

const char *CommandEntry::getName() const {
    return this->name;
}

void CommandEntry::setName(const char *name) {
    this->name = name;
}

commandProc CommandEntry::getProc() const {
    return this->proc;
}

void CommandEntry::setProc(commandProc proc) {
    this->proc = proc;
}

int CommandEntry::getArity() const {
    return this->arity;
}

void CommandEntry::setArity(int arity) {
    this->arity = arity;
}

const std::string &CommandEntry::getSflags() const {
    return this->sflags;
}

void CommandEntry::setSflags(const std::string &sflags) {
    this->sflags = sflags;
}

int CommandEntry::getFlag() const {
    return this->flag;
}

void CommandEntry::setFlag(int flag) {
    this->flag = flag;
}

void CommandEntry::addFlag(int flag) {
    this->flag |= flag;
}

bool CommandEntry::IsWrite() const {
    return this->flag & CMD_WRITE;
}

void CommandEntry::setKeysProc(getKeysProc proc) {
    this->keysProc = keysProc;
}

uint64_t CommandEntry::getMicroseconds() const {
    return this->microseconds;
}

void CommandEntry::setMicroseconds(uint64_t microseconds) {
    this->microseconds = microseconds;
}

uint64_t CommandEntry::getCalls() const {
    return this->calls;
}

void CommandEntry::setCalls(uint64_t calls) {
    this->calls = calls;
}

int CommandEntry::getFirstKey() const {
    return firstKey;
}

void CommandEntry::setFirstKey(int firstKey) {
    CommandEntry::firstKey = firstKey;
}

int CommandEntry::getLastKey() const {
    return lastKey;
}

void CommandEntry::setLastKey(int lastKey) {
    CommandEntry::lastKey = lastKey;
}

int CommandEntry::getKeyStep() const {
    return keyStep;
}

void CommandEntry::setKeyStep(int keyStep) {
    CommandEntry::keyStep = keyStep;
}

void versionCommand(const AbstractCoordinator* coordinator,
                    AbstractFlyClient *client) {
    if (NULL == client) {
        return;
    }

    client->addReply("FlyDB version: %s",
                     coordinator->getFlyServer()->getVersion().c_str());
}

void getCommand(const AbstractCoordinator* coordinator,
                AbstractFlyClient* flyClient) {
    if (NULL == flyClient) {
        return;
    }

    // 如果参数数量<2，直接返回
    if (flyClient->getArgc() < 2) {
        flyClient->addReply("missing parameters!");
        return;
    }

    // 获取到key
    std::string *key = reinterpret_cast<std::string*>(
            flyClient->getArgv()[1]->getPtr());

    // 查看key是否已经过期
    AbstractFlyDB *flyDB = flyClient->getFlyDB();
    uint64_t expireTime = flyDB->getExpire(*key);
    if (expireTime != -1 && expireTime < time(NULL)) {
        flyDB->delKey(*key);
        // todo: add delete command to feedAppendOnlyFile
    }

    // 返回结果
    std::string* val = reinterpret_cast<std::string*>(
            flyDB->lookupKey(*key)->getPtr());
    flyClient->addReply(val->c_str(), val->length());
}

void setGenericCommand(const AbstractCoordinator *coordinator,
                       AbstractFlyClient *flyClient,
                       std::string *key,
                       std::shared_ptr<FlyObj> val,
                       int64_t expireMilli) {
    // 将key和val添加到flydb中
    if (-1 == flyClient->getFlyDB()->addExpire(*key, val, expireMilli)) {
        flyClient->addReply("set error!");
        return;
    }

    coordinator->getFdbHandler()->addDirty(1);
    flyClient->addReply("set OK!");
}

void setCommand(const AbstractCoordinator* coordinator,
                AbstractFlyClient* flyClient) {
    if (NULL == flyClient) {
        return;
    }

    // 如果参数数量 < 2，直接返回
    if (flyClient->getArgc() < 3) {
        flyClient->addReply("missing parameters!");
        return;
    }

    // 获取到key和val
    std::string *key = reinterpret_cast<std::string*>
    (flyClient->getArgv()[1]->getPtr());
    std::shared_ptr<FlyObj> val = flyClient->getArgv()[2];

    setGenericCommand(coordinator, flyClient, key, val, -1);
}

void setExCommand(const AbstractCoordinator* coordinator,
                  AbstractFlyClient* flyClient) {
    if (NULL == flyClient) {
        return;
    }

    // 如果参数数量 < 3，直接返回
    if (flyClient->getArgc() < 4) {
        flyClient->addReply("missing parameters!");
        return;
    }

    // 获取到key和val
    std::string *key =
            reinterpret_cast<std::string*>(flyClient->getArgv()[1]->getPtr());
    std::shared_ptr<FlyObj> val = flyClient->getArgv()[2];

    // 获取超时时间
    int64_t expireSeconds =
            *(reinterpret_cast<int*>(flyClient->getArgv()[3]->getPtr()));
    int64_t expireMilli = (-1 == expireSeconds ? -1 : expireSeconds * 1000);

    setGenericCommand(coordinator, flyClient, key, val, expireMilli);
}

void psetExCommand(const AbstractCoordinator* coordinator,
                   AbstractFlyClient* flyClient) {
    if (NULL == flyClient) {
        return;
    }

    // 如果参数数量 < 3，直接返回
    if (flyClient->getArgc() < 4) {
        flyClient->addReply("missing parameters!");
        return;
    }

    // 获取到key和val
    std::string *key =
            reinterpret_cast<std::string*>(flyClient->getArgv()[1]->getPtr());
    std::shared_ptr<FlyObj> val = flyClient->getArgv()[2];
    int64_t expireMilli =
            *(reinterpret_cast<int*>(flyClient->getArgv()[3]->getPtr()));

    setGenericCommand(coordinator, flyClient, key, val, expireMilli);
}

void expireCommand(const AbstractCoordinator* coordinator,
                   AbstractFlyClient* flyClient) {
}

void expireatCommand(const AbstractCoordinator* coordinator,
                     AbstractFlyClient* flyClient) {

}

void mgetCommand(const AbstractCoordinator* coordinator,
                 AbstractFlyClient* flyClient) {

}

enum ListLocation {
    LIST_HEAD,
    LIST_TAIL
};

void pushGenericCommand(const AbstractCoordinator* coordinator,
                        AbstractFlyClient* flyClient,
                        ListLocation location) {
    if (NULL == flyClient) {
        return;
    }

    // 如果参数数量 < 2，直接返回
    if (flyClient->getArgc() < 3) {
        flyClient->addReply("missing parameters!");
        return;
    }

    // 获取到list
    std::string *key =
            reinterpret_cast<std::string*>(flyClient->getArgv()[1]->getPtr());
    AbstractFlyDB *flyDB = flyClient->getFlyDB();
    std::list<std::string> *list = reinterpret_cast<std::list<std::string> *>(
            flyDB->lookupKey(*key)->getPtr());
    if (NULL == list) {
        std::shared_ptr<FlyObj> obj =
                coordinator->getFlyObjLinkedListFactory()->getObject();
        flyDB->add(*key, obj);
    }

    for (int j = 2; j < flyClient->getArgc(); j++) {
        if (LIST_HEAD == location) {
            list->push_front(*reinterpret_cast<std::string*>
                             (flyClient->getArgv()[j]->getPtr()));
        } else {
            list->push_back(*reinterpret_cast<std::string*>
                            (flyClient->getArgv()[j]->getPtr()));
        }
    }

    coordinator->getFdbHandler()->addDirty(flyClient->getArgc() - 2);
    flyClient->addReply("push OK!");
}

void rpushCommand(const AbstractCoordinator* coordinator,
                  AbstractFlyClient* flyClient) {
    pushGenericCommand(coordinator, flyClient, LIST_HEAD);
}

void lpushCommand(const AbstractCoordinator* coordinator,
                  AbstractFlyClient* flyClient) {
    pushGenericCommand(coordinator, flyClient, LIST_TAIL);
}

void pushSortCommand(const AbstractCoordinator* coordinator,
                     AbstractFlyClient *flyClient) {
    if (NULL == flyClient) {
        return;
    }

    // 如果参数数量 < 2，直接返回
    if (flyClient->getArgc() < 3) {
        flyClient->addReply("missing parameters!");
        return;
    }

    // 获取到list
    std::string *key =
            reinterpret_cast<std::string*>(flyClient->getArgv()[1]->getPtr());
    AbstractFlyDB *flyDB = flyClient->getFlyDB();
    SkipList<std::string> *list = reinterpret_cast<SkipList<std::string> *>
    (flyDB->lookupKey(*key)->getPtr());
    if (NULL == list) {
        std::shared_ptr<FlyObj> obj = coordinator->getFlyObjLinkedListFactory()
                ->getObject(list = new SkipList<std::string>());
        flyDB->add(*key, obj);
    }

    for (int j = 2; j < flyClient->getArgc(); j++) {
        list->insertNode(*(reinterpret_cast<std::string*>(
                flyClient->getArgv()[j]->getPtr())));
    }

    coordinator->getFdbHandler()->addDirty(flyClient->getArgc() - 2);
    flyClient->addReply("push OK!");
}

void popSortCommand(const AbstractCoordinator* coordinator,
                    AbstractFlyClient *flyClient) {
    if (NULL == flyClient) {
        return;
    }

    // 如果参数数量 < 2，直接返回
    if (flyClient->getArgc() < 3) {
        flyClient->addReply("missing parameters!");
        return;
    }

    std::string *key =
            reinterpret_cast<std::string *>(flyClient->getArgv()[1]->getPtr());
    std::shared_ptr<FlyObj> val = flyClient->getFlyDB()->lookupKey(*key);
    if (NULL == val) {
        flyClient->addReply("don`t have key: %s", key);
    }

    SkipList<std::string> *list =
            reinterpret_cast<SkipList<std::string> *>(val->getPtr());
    list->deleteNode(
            *(reinterpret_cast<std::string *>(flyClient->getArgv()[3]->getPtr())));

    coordinator->getFdbHandler()->addDirty(1);
    flyClient->addReply("status OK!");
}

void popGenericCommand(const AbstractCoordinator *coordinator,
                       AbstractFlyClient *flyClient,
                       ListLocation location) {
    if (NULL == flyClient) {
        return;
    }

    // 如果参数数量 < 2，直接返回
    if (flyClient->getArgc() < 2) {
        flyClient->addReply("missing parameters!");
        return;
    }

    std::string *key =
            reinterpret_cast<std::string *>(flyClient->getArgv()[1]->getPtr());
    std::shared_ptr<FlyObj> val = flyClient->getFlyDB()->lookupKey(*key);
    if (NULL == val) {
        flyClient->addReply("don`t have key: %s", key);
    }

    std::list<std::string *> *list =
            reinterpret_cast<std::list<std::string *> *>(val->getPtr());
    if (LIST_HEAD == location) {
        list->pop_front();
    } else {
        list->pop_back();
    }

    coordinator->getFdbHandler()->addDirty(1);
    flyClient->addReply("status OK!");
}

void rpopCommand(const AbstractCoordinator* coordinator,
                 AbstractFlyClient* flyClient) {
    popGenericCommand(coordinator, flyClient, LIST_HEAD);
}

void lpopCommand(const AbstractCoordinator* coordinator,
                 AbstractFlyClient* flyClient) {
    popGenericCommand(coordinator, flyClient, LIST_TAIL);
}

void hsetCommand(const AbstractCoordinator* coordinator,
                 AbstractFlyClient* flyClient) {
    if (NULL == flyClient) {
        return;
    }

    // 如果参数数量 != 2n，直接返回
    uint8_t argc = flyClient->getArgc();
    if (argc < 2 || argc % 2 == 1) {
        flyClient->addReply("parameters error!");
        return;
    }

    std::string *table =
            reinterpret_cast<std::string *>(flyClient->getArgv()[1]->getPtr());
    std::shared_ptr<FlyObj> val = flyClient->getFlyDB()->lookupKey(*table);
    if (NULL == val) {
        val = coordinator->getFlyObjHashTableFactory()->getObject();
    }

    Dict<std::string, std::string> *dict =
            reinterpret_cast<Dict<std::string, std::string> *>(val->getPtr());
    for (int i = 2; i < argc; i = i + 2) {
        std::string *key = reinterpret_cast<std::string *>(
                flyClient->getArgv()[i]->getPtr());
        std::string *val = reinterpret_cast<std::string *>(
                flyClient->getArgv()[i + 1]->getPtr());
        dict->addEntry(*key, *val);
    }

    coordinator->getFdbHandler()->addDirty(argc / 2 - 1);
    flyClient->addReply("status OK!");
}

void hgetCommand(const AbstractCoordinator* coordinator,
                 AbstractFlyClient* flyClient) {
    if (NULL == flyClient) {
        return;
    }

    // 如果参数数量 < 2，直接返回
    if (flyClient->getArgc() < 3) {
        flyClient->addReply("missing parameters!");
        return;
    }

    std::string *tableName =
            reinterpret_cast<std::string *>(flyClient->getArgv()[1]->getPtr());
    std::shared_ptr<FlyObj> table =
            flyClient->getFlyDB()->lookupKey(*tableName);
    if (NULL == table) {
        flyClient->addReply("Don`t have this talbe: %s", tableName);
        return;
    }

    Dict<std::string, std::string> *dict =
            reinterpret_cast<Dict<std::string, std::string> *>(table->getPtr());
    std::string *key =
            reinterpret_cast<std::string *>(flyClient->getArgv()[2]->getPtr());
    std::string val;
    int res = dict->fetchValue(*key, &val);
    if (-1 == res) {
        flyClient->addReply("Don`t have key : %s", key);
        return;
    }

    flyClient->addReply("value: %s", val.c_str());
    return;
}

void saveCommand(const AbstractCoordinator* coordinator,
                 AbstractFlyClient* flyClient) {
    AbstractFDBHandler *fdbHandler = coordinator->getFdbHandler();
    fdbHandler->save();
}

/** BGSAVE [SCHEDULE] */
void bgsaveCommand(const AbstractCoordinator* coordinator,
                   AbstractFlyClient* flyClient) {
    if (NULL == flyClient) {
        return;
    }

    AbstractFlyServer *flyServer = coordinator->getFlyServer();

    /** 获取schedule */
    bool schedule = false;
    if (2 == flyClient->getArgc()) {
        std::string *argv1 = reinterpret_cast<std::string *>(
                flyClient->getArgv()[1]->getPtr());
        if (0 == argv1->compare("schedule")) {
            schedule = true;
        } else {
            flyClient->addReply("wrong parameters");
            return;
        }
    }

    /** 如果有fdb持久化子进程存在，则说明处于fdb过程中，不允许再次执行fdb */
    if (coordinator->getFdbHandler()->haveChildPid()) {
        flyClient->addReply("Background save already in progress");
        return;
    }

    /**
     * 如果有aof持久化子进程存在，则说明处于fdb过程中，
     * 如果是执行fdb子进程调度，则标记schdule flag, 以便于后续在serverCron函数中执行fdb操作
     * 否则，如果是直接进行fdb, 则不允许执行
     **/
    if (coordinator->getAofHandler()->haveChildPid()) {
        if (!schedule) {
            flyClient->addReply("An AOF log rewriting in progress: can't BGSAVE"
                                " right now. Use BGSAVE SCHEDULE in order to "
                                "schedule a BGSAVE whenever possible.");
            return;
        } else {
            coordinator->getFdbHandler()->setBGSaveScheduled(true);
            flyClient->addReply("Background saving scheduled");
            return;
        }
    }
    if (coordinator->getFdbHandler()->backgroundSave() > 0) {
        flyClient->addReply("Background saving started");
    } else {
        flyClient->addReply("error to do fdb");
    }

    return;
}

void bgrewriteaofCommand(const AbstractCoordinator *coordinator,
                         AbstractFlyClient *flyClient) {
    /** have aof background thread */
    if (coordinator->getAofHandler()->haveChildPid()) {
        flyClient->addReply(
                "Background append only file rewriting already in progress");
        return;
    }

    /** have fdb background thread, then schedule this aof command */
    if (coordinator->getFdbHandler()->haveChildPid()) {
        flyClient->addReply("Background append only file rewriting scheduled");
        coordinator->getFdbHandler()->setBGSaveScheduled(true);
        return;
    }

    if (coordinator->getAofHandler()->start() > 0) {
        flyClient->addReply("Background append only file rewriting started");
    } else {
        flyClient->addReply("Background append only file rewriting error!");
    }

    return;
}


void configSetCommand(const AbstractCoordinator* coordinator,
                      AbstractFlyClient* flyClient) {
    if (flyClient->getArgc() != 4) {
        return;
    }

    std::string *argv2 = reinterpret_cast<std::string*>(
            flyClient->getArgv()[2]->getPtr());
    std::string *argv3 = reinterpret_cast<std::string*>(
            flyClient->getArgv()[3]->getPtr());

    if (0 == argv2->compare("appendonly")) {
        AbstractAOFHandler *aofHandler = coordinator->getAofHandler();
        int enable = 0;
        enable = miscTool->yesnotoi(argv3->c_str());
        if (0 == enable && !aofHandler->IsStateOff()) {
            aofHandler->stop();
        } else if (1 == enable && aofHandler->IsStateOff()) {
            if (-1 == aofHandler->start()) {
                flyClient->addReply(
                        "Unable to turn on AOF. Check server logs.");
                return;
            }
        }
    }

    return;
}

void configGetCommand(const AbstractCoordinator* coordinator,
                      AbstractFlyClient* flyClient) {

}

void configCommand(const AbstractCoordinator* coordinator,
                   AbstractFlyClient* flyClient) {
    if (flyClient->getArgc() < 2) {
        return;
    }

    std::string *argv1 = reinterpret_cast<std::string*>(
            flyClient->getArgv()[1]->getPtr());

    if (coordinator->getFlyServer()->isLoading() && argv1->compare("get")) {
        flyClient->addReply("Only CONFIG GET is allowed during loading");
        return;
    }

    if (0 == argv1->compare("set")) {
        if (4 != flyClient->getArgc()) {
            flyClient->addReply("Wrong number of arguments for CONFIG %s",
                                argv1->c_str());
            return;
        }
        configSetCommand(coordinator, flyClient);
    } else if (0 == argv1->compare("get")) {
        if (3 != flyClient->getArgc()) {
            flyClient->addReply("Wrong number of arguments for CONFIG %s",
                                argv1->c_str());
            return;
        }
        configGetCommand(coordinator, flyClient);
    }

    return;
}

void selectCommand(const AbstractCoordinator* coordinator,
                   AbstractFlyClient* flyClient) {
    if (flyClient->getArgc() < 2) {
        return;
    }

    std::string *argv1 = reinterpret_cast<std::string*>(
            flyClient->getArgv()[1]->getPtr());
    int64_t num = 0;
    if (-1 == miscTool->string2int64(*argv1, num)) {
        coordinator->getLogHandler()->logNotice(
                "Wrong parammeter type: %s", argv1);
       return;
    }

    AbstractFlyDB *flyDB = coordinator->getFlyServer()->getFlyDB(num);
    if (NULL == flyDB) {
        coordinator->getLogHandler()->logNotice(
                "Can`t get flyDB for db num: %d", num);
        return;
    }
    flyClient->setFlyDB(flyDB);
    flyClient->setDbid(num);
}
