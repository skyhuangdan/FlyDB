//
// Created by 赵立伟 on 2018/10/18.
//

#include "FlyClient.h"
#include "../net/NetDef.h"
#include "ClientDef.h"
#include "../FlyServer.h"

FlyClient::FlyClient(int fd, FlyServer *flyServer) {
    this->flyServer = flyServer;
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
    this->bufpos = 0;
    this->reqType = 0;
    this->replyBytes = 0;
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

void FlyClient::addFlag(int flag) {
    this->flags |= flag;
}

FlyObj **FlyClient::getArgv() const {
    return argv;
}

void FlyClient::freeArgv() const {
    if (NULL != argv) {
        delete[] argv;
    }
}

void FlyClient::allocArgv(int64_t multiBulkLen) {
    this->argv = new FlyObj*[multiBulkLen];
}

void FlyClient::addArgv(FlyObj *obj) {
    this->argv[this->argc++] = obj;
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

const std::list<std::string*> &FlyClient::getReply() const {
    return replies;
}

void FlyClient::setReply(const std::list<std::string*> &reply) {
    this->replies = reply;
}

const std::string &FlyClient::getQueryBuf() const {
    return queryBuf;
}

void FlyClient::addToQueryBuf(const std::string &str) {
    this->queryBuf += str;
}

int FlyClient::getQueryBufSize() const {
    return this->queryBuf.length();
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

bool FlyClient::isMultiBulkType() {
    return '*' == this->queryBuf[0];
}

int32_t FlyClient::getMultiBulkLen() const {
    return multiBulkLen;
}

void FlyClient::setMultiBulkLen(int32_t multiBulkLen) {
    this->multiBulkLen = multiBulkLen;
}

void FlyClient::setQueryBuf(const std::string &queryBuf) {
    this->queryBuf = queryBuf;
}

void FlyClient::trimQueryBuf(int begin, int end) {
    std::string sub = queryBuf.substr(begin, end);
    setQueryBuf(sub);
}

int64_t FlyClient::getBulkLen() const {
    return bulkLen;
}

void FlyClient::setBulkLen(int64_t bulkLen) {
    FlyClient::bulkLen = bulkLen;
}

bool FlyClient::hasNoPending() {
    return !(this->bufpos > 0 || this->replies.size() > 0) && !(this->flags & CLIENT_PENDING_WRITE);
}

int FlyClient::prepareClientToWrite() {
    if (this->fd < 0) {
        return -1;
    }

    // 如果之前没有写入，说明write handler不存在，需要先将其标记并放入flyserver的pending client list中
    if (hasNoPending()) {
        this->addFlag(CLIENT_PENDING_WRITE);
        this->flyServer->addToClientsPendingToWrite(this);
    }

    return 1;
}

void FlyClient::addReply(const char *s, size_t len) {
    // 查看client状态是否可以写入
    if (-1 == prepareClientToWrite()) {
        return;
    }

    // 如果写入固定缓冲区失败，则向可变缓冲区写入
    if (-1 == addReplyToBuffer(s, len)) {
        addReplyToReplyList(s, len);
    }
}

int FlyClient::addReplyToBuffer(const char *s, size_t len) {
    // 无需写入
    if (this->flags & CLIENT_CLOSE_AFTER_REPLY) {
        return 1;
    }

    // 如果可变缓冲区中有数据，则继续往可变缓冲区写入
    if (this->replies.size() > 0) {
        return -1;
    }

    // 空间不足，无法写入
    if (sizeof(this->buf) - this->bufpos < len) {
        return -1;
    }

    // 写入固定缓冲区
    memcpy(this->buf + this->bufpos, s, len);
    this->bufpos += len;

    return 1;
}

int FlyClient::addReplyToReplyList(const char *s, size_t len) {
    // 无需写入
    if (this->flags & CLIENT_CLOSE_AFTER_REPLY) {
        return 1;
    }

    std::string *reply = this->replies.back();
    if (NULL != reply && strlen((*reply).c_str()) + len <= PROTO_REPLY_CHUNK_BYTES) {
        (*reply) += s;
    } else {
        reply = new std::string(s);
        this->replies.push_back(reply);
    }
    this->replyBytes += len;
}

