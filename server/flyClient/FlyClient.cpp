//
// Created by 赵立伟 on 2018/10/18.
//

#include "FlyClient.h"

int FlyClient::getFd() const {
    return fd;
}

void FlyClient::setFd(int fd) {
    FlyClient::fd = fd;
}

FlyObj *FlyClient::getName() const {
    return name;
}

void FlyClient::setName(FlyObj *name) {
    FlyClient::name = name;
}

int FlyClient::getFlags() const {
    return flags;
}

void FlyClient::setFlags(int flags) {
    FlyClient::flags = flags;
}

const std::string &FlyClient::getQueryBuf() const {
    return queryBuf;
}

void FlyClient::setQueryBuf(const std::string &queryBuf) {
    FlyClient::queryBuf = queryBuf;
}

FlyObj **FlyClient::getArgv() const {
    return argv;
}

void FlyClient::setArgv(FlyObj **argv) {
    FlyClient::argv = argv;
}

int FlyClient::getArgc() const {
    return argc;
}

void FlyClient::setArgc(int argc) {
    FlyClient::argc = argc;
}

CommandEntry *FlyClient::getCmd() const {
    return cmd;
}

void FlyClient::setCmd(CommandEntry *cmd) {
    FlyClient::cmd = cmd;
}

const char *FlyClient::getBuf() const {
    return buf;
}

int FlyClient::getBufSize() const {
    return bufSize;
}

void FlyClient::setBufSize(int bufSize) {
    FlyClient::bufSize = bufSize;
}

std::list<std::string> *FlyClient::getReply() const {
    return reply;
}

void FlyClient::setReply(std::list<std::string> *reply) {
    FlyClient::reply = reply;
}

int FlyClient::getAuthentiated() const {
    return authentiated;
}

void FlyClient::setAuthentiated(int authentiated) {
    FlyClient::authentiated = authentiated;
}

time_t FlyClient::getCreateTime() const {
    return createTime;
}

void FlyClient::setCreateTime(time_t createTime) {
    FlyClient::createTime = createTime;
}

time_t FlyClient::getLastInteractionTime() const {
    return lastInteractionTime;
}

void FlyClient::setLastInteractionTime(time_t lastInteractionTime) {
    FlyClient::lastInteractionTime = lastInteractionTime;
}

time_t FlyClient::getSoftLimitTime() const {
    return softLimitTime;
}

void FlyClient::setSoftLimitTime(time_t softLimitTime) {
    FlyClient::softLimitTime = softLimitTime;
}
