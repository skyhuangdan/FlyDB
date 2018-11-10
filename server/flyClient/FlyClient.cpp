//
// Created by 赵立伟 on 2018/10/18.
//

#include "FlyClient.h"
#include "../net/NetDef.h"

FlyClient::FlyClient(int fd) {
    this->fd = fd;
    this->name = NULL;
    this->flags = 0;
    this->argc = 0;
    this->argv = NULL;
    this->cmd = NULL;
    this->authentiated = 0;
    this->createTime = this->lastInteractionTime = time(NULL);
    this->softLimitTime = 0;
    this->buf = new char[FLY_REPLY_CHUNK_BYTES];
}

FlyClient::~FlyClient() {
    delete[] this->buf;
}

int FlyClient::getFd() const {
    return fd;
}

void FlyClient::setFd(int fd) {
    this->fd = fd;
}

FlyObj *FlyClient::getName() const {
    return name;
}

void FlyClient::setName(FlyObj *name) {
    this->name = name;
}

int FlyClient::getFlags() const {
    return flags;
}

void FlyClient::setFlags(int flags) {
    this->flags = flags;
}

FlyObj **FlyClient::getArgv() const {
    return argv;
}

void FlyClient::setArgv(FlyObj **argv) {
    this->argv = argv;
}

int FlyClient::getArgc() const {
    return argc;
}

void FlyClient::setArgc(int argc) {
    this->argc = argc;
}

CommandEntry *FlyClient::getCmd() const {
    return cmd;
}

void FlyClient::setCmd(CommandEntry *cmd) {
    this->cmd = cmd;
}

int FlyClient::getAuthentiated() const {
    return authentiated;
}

void FlyClient::setAuthentiated(int authentiated) {
    this->authentiated = authentiated;
}

time_t FlyClient::getCreateTime() const {
    return createTime;
}

void FlyClient::setCreateTime(time_t createTime) {
    this->createTime = createTime;
}

time_t FlyClient::getLastInteractionTime() const {
    return lastInteractionTime;
}

void FlyClient::setLastInteractionTime(time_t lastInteractionTime) {
    this->lastInteractionTime = lastInteractionTime;
}

time_t FlyClient::getSoftLimitTime() const {
    return softLimitTime;
}

void FlyClient::setSoftLimitTime(time_t softLimitTime) {
    this->softLimitTime = softLimitTime;
}

const std::list<std::string> &FlyClient::getReply() const {
    return reply;
}

void FlyClient::setReply(const std::list<std::string> &reply) {
    this->reply = reply;
}

const std::string &FlyClient::getQueryBuf() const {
    return queryBuf;
}

void FlyClient::setQueryBuf(const std::string &queryBuf) {
    this->queryBuf = queryBuf;
}

uint64_t FlyClient::getId() const {
    return id;
}

void FlyClient::setId(uint64_t id) {
    this->id = id;
}

char *FlyClient::getBuf() const {
    return buf;
}

void FlyClient::setBuf(char *buf) {
    this->buf = buf;
}

