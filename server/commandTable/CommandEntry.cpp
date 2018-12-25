//
// Created by 赵立伟 on 2018/9/19.
//

#include <iostream>
#include "CommandEntry.h"
#include "../flyServer/interface/AbstractFlyServer.h"

void versionCommand(AbstractFlyServer* server, AbstractFlyClient *client) {
    if (NULL == server || NULL == client) {
        return;
    }

    char buf[1024];
    snprintf(buf, sizeof(buf), "FlyDB version: %s",
             server->getVersion().c_str());
    client->addReply(buf, strlen(buf));
}

void getCommand(AbstractFlyServer* flyServer,
                AbstractFlyClient* flyClient) {
    if (NULL == flyClient) {
        return;
    }

    // 如果参数数量<2，直接返回
    if (flyClient->getArgc() < 2) {
        char buf[100];
        snprintf(buf, sizeof(buf), "missing parameters!");
        flyClient->addReply(buf, strlen(buf));
        return;
    }

    // 获取到key
    std::string *key = reinterpret_cast<std::string*>
    (flyClient->getArgv()[1]->getPtr());

    // 查看key是否已经过期
    AbstractFlyDB *flyDB = flyClient->getFlyDB();
    uint64_t expireTime = flyDB->getExpire(key);
    if (expireTime != -1 && expireTime < time(NULL)) {
        flyDB->delKey(key);
    }

    // 返回结果
    std::string* val = reinterpret_cast<std::string*>(
            flyDB->lookupKey(key)->getPtr());
    flyClient->addReply(val->c_str(), val->length());
}

void setCommand(AbstractFlyServer* flyServer,
                AbstractFlyClient* flyClient) {
    if (NULL == flyClient) {
        return;
    }

    // 如果参数数量<2，直接返回
    if (flyClient->getArgc() < 3) {
        char buf[100];
        snprintf(buf, sizeof(buf), "missing parameters!");
        flyClient->addReply(buf, strlen(buf));
        return;
    }

    // 获取到key和val
    std::string *key = reinterpret_cast<std::string*>
    (flyClient->getArgv()[1]->getPtr());
    FlyObj *val = flyClient->getArgv()[2];

    // 将key和val添加到flydb中
    flyClient->getFlyDB()->add(key, val);

    char buf[1024];
    snprintf(buf, sizeof(buf), "status OK: key-%s, val-%s", key, val->getPtr());
    flyClient->addReply(buf, strlen(buf));
}

void expireCommand(AbstractFlyServer* flyServer,
                   AbstractFlyClient* flyClient) {

}

void expireatCommand(AbstractFlyServer* flyServer,
                     AbstractFlyClient* flyClient) {

}

void mgetCommand(AbstractFlyServer* flyServer,
                 AbstractFlyClient* flyClient) {

}

void rpushCommand(AbstractFlyServer* flyServer,
                  AbstractFlyClient* flyClient) {

}

void lpushCommand(AbstractFlyServer* flyServer,
                  AbstractFlyClient* flyClient) {

}

void lpushxCommand(AbstractFlyServer* flyServer,
                   AbstractFlyClient* flyClient) {

}

void linsertCommand(AbstractFlyServer* flyServer,
                    AbstractFlyClient* flyClient) {

}

void rpopCommand(AbstractFlyServer* flyServer,
                 AbstractFlyClient* flyClient) {

}

void lpopCommand(AbstractFlyServer* flyServer,
                 AbstractFlyClient* flyClient) {

}

void brpopCommand(AbstractFlyServer* flyServer,
                  AbstractFlyClient* flyClient) {

}

void hsetCommand(AbstractFlyServer* flyServer,
                 AbstractFlyClient* flyClient) {

}

void hmgetCommand(AbstractFlyServer* flyServer,
                  AbstractFlyClient* flyClient) {

}

void saveCommand(AbstractFlyServer* flyServer,
                 AbstractFlyClient* flyClient) {

}

void bgsaveCommand(AbstractFlyServer* flyServer,
                   AbstractFlyClient* flyClient) {

}

char *CommandEntry::getName() const {
    return this->name;
}

void CommandEntry::setName(char *name) {
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

void CommandEntry::setKeysProc(getKeysProc proc) {
    this->keysProc = keysProc;
}

bool CommandEntry::isFirstKey() const {
    return this->firstKey;
}

void CommandEntry::setFirstKey(bool firstKey) {
    this->firstKey = firstKey;
}

bool CommandEntry::isLastKey() const {
    return this->lastKey;
}

void CommandEntry::setLastKey(bool lastKey) {
    this->lastKey = lastKey;
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
