//
// Created by 赵立伟 on 2018/10/18.
//

#include "FlyClient.h"
#include "../net/NetDef.h"
#include "ClientDef.h"
#include "../io/StringFio.h"

FlyClient::FlyClient(int fd,
                     const AbstractCoordinator *coordinator,
                     AbstractFlyDB *flyDB) {
    this->coordinator = coordinator;
    this->id = 0;
    this->fd = fd;
    this->name = NULL;
    this->flags = 0;
    this->argc = 0;
    this->argv = NULL;
    this->cmd = NULL;
    this->authentiated = 0;
    this->createTime = this->lastInteractionTime = time(NULL);
    this->softLimitTime = 0;
    this->bufpos = 0;
    this->replyBytes = 0;
    this->reqType = 0;
    this->multiBulkLen = 0;
    this->bulkLen = 0;
    this->sendLen = 0;
    this->flyDB = flyDB;
}

FlyClient::~FlyClient() {
}

int FlyClient::getFd() const {
    return fd;
}

void FlyClient::setFd(int fd) {
    this->fd = fd;
}

std::shared_ptr<FlyObj> FlyClient::getName() const {
    return name;
}

void FlyClient::setName(std::shared_ptr<FlyObj> name) {
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

void FlyClient::delFlag(int flag) {
    this->flags &= ~flag;
}

std::shared_ptr<FlyObj> *FlyClient::getArgv() const {
    return this->argv;
}

void FlyClient::freeArgv() {
    if (NULL != this->argv) {
        for (int i = 0; i < this->argc; i++) {
            this->argv[i] = NULL;
        }
        delete[] this->argv;
        this->argv = NULL;
    }
    this->argc = 0;
}

void FlyClient::allocArgv(int64_t count) {
    this->argv = new std::shared_ptr<FlyObj> [count];
}

void FlyClient::addArgv(std::shared_ptr<FlyObj> obj) {
    this->argv[this->argc++] = obj;
}

int FlyClient::getArgc() const {
    return argc;
}

void FlyClient::setArgc(int argc) {
    this->argc = argc;
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
    return this->replies;
}

void FlyClient::replyPopFront() {
    this->replyBytes -= this->replies.front()->size();
    this->replies.pop_front();
    if (0 == this->replies.size() && 0 != this->replyBytes) {
        this->logHandler->logVerbose("The reply size is zero, "
                                     "but replyBytes is not: %d",
                                     this->replyBytes);
        this->replyBytes = 0;
    }
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
    return this->id;
}

const char *FlyClient::getBuf() const {
    return this->buf;
}

void FlyClient::clearBuf(){
    this->setBufpos(0);
    this->setSendLen(0);
}

bool FlyClient::bufSendOver() {
    return this->sendLen == this->bufpos;
}

void FlyClient::setId(uint64_t id) {
    this->id = id;
}

char FlyClient::getFirstQueryChar() {
    return this->queryBuf[0];
}

bool FlyClient::isMultiBulkType() {
    return PROTO_REQ_MULTIBULK == this->reqType;
}

int32_t FlyClient::getMultiBulkLen() const {
    return this->multiBulkLen;
}

void FlyClient::setMultiBulkLen(int32_t multiBulkLen) {
    this->multiBulkLen = multiBulkLen;
}

void FlyClient::setQueryBuf(const std::string &queryBuf) {
    this->queryBuf = queryBuf;
}

void FlyClient::trimQueryBuf(int begin, int end) {
    std::string sub;
    if (-1 == end) {
        sub = queryBuf.substr(begin);
    } else {
        sub = queryBuf.substr(begin, end);
    }

    setQueryBuf(sub);
}

int64_t FlyClient::getBulkLen() const {
    return bulkLen;
}

void FlyClient::setBulkLen(int64_t bulkLen) {
    this->bulkLen = bulkLen;
}

bool FlyClient::hasNoPending() {
    return 0 == this->bufpos && 0 == this->replies.size();
}

int FlyClient::prepareClientToWrite() {
    if (this->fd < 0) {
        return -1;
    }

    /**
     * 如果当前client是主服务器，则该主服务器不接收通知。
     * 除非设置了CLIENT_MASTER_FORCE_REPLY
     **/
    if (this->flags & CLIENT_MASTER
        && !(this->flags & CLIENT_MASTER_FORCE_REPLY)) {
        return -1;
    }

    /** 清空force reply标志位 */
    this->delFlag(CLIENT_MASTER_FORCE_REPLY);

    /**
     * 如果之前没有写入，说明write handler不存在，
     * flyserver也不存在pending client list中,
     * 需要先将其标记并放入flyserver的pending client list中
     */
    if (hasNoPending() && !(this->flags & CLIENT_PENDING_WRITE)) {
        this->addFlag(CLIENT_PENDING_WRITE);
        this->coordinator->getFlyServer()->addToClientsPendingToWrite(
                std::shared_ptr<AbstractFlyClient>(this));
    }

    return 1;
}

bool FlyClient::IsPendingWrite() const {
    return this->flags & CLIENT_PENDING_WRITE;
}

int FlyClient::getBufpos() const {
    return bufpos;
}

void FlyClient::setBufpos(int bufpos) {
    this->bufpos = bufpos;
}

void FlyClient::addReplyRaw(const char *s) {
    this->addReply(s, strlen(s));
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

void FlyClient::addReply(const char *fmt, ...) {
    char msg[LOG_MAX_LEN];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);

    this->addReplyRaw(msg);
}

void FlyClient::addReplyErrorFormat(const char *fmt, ...) {
    va_list ap;
    char msg[1024];
    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);

    int len = strlen(msg);
    // 保证msg中没有换行符, 使msg在一行内
    for (int i = 0; i < len; i++) {
        if ('\r' == msg[i] || '\n' == msg[i]) {
            msg[i] = ' ';
        }
    }

    addReplyError(msg);
}

void FlyClient::addReplyError(const char *err) {
    std::shared_ptr<StringFio> fio =
            std::shared_ptr<StringFio>(new StringFio());
    fio->writeBulkError(err);
    this->addReply(fio->getStr().c_str());
}

void FlyClient::addReplyBulkCount(int count) {
    if (count < OBJ_SHARED_BULKHDR_LEN) {
        extern std::shared_ptr<FlyObj> mbulkHeader[OBJ_SHARED_BULKHDR_LEN];
        std::string *str = reinterpret_cast<std::string *>(
                mbulkHeader[count]->getPtr());
        this->addReply(str->c_str());
    } else {
        std::shared_ptr<StringFio> fio =
                std::shared_ptr<StringFio>(new StringFio());
        fio->writeBulkCount('*', count);
        this->addReply(fio->getStr().c_str());
    }

    return;
}

void FlyClient::addReplyBulkString(std::string str) {
    StringFio *fio = new StringFio();
    fio->writeBulkString(str);
    this->addReply(fio->getStr().c_str());
    delete fio;
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

    // todo: 检查是否达到soft limit和hard limit

    return 1;
}

int FlyClient::getReqType() const {
    return this->reqType;
}

void FlyClient::setReqType(int reqType) {
    this->reqType = reqType;
}

size_t FlyClient::getSendLen() const {
    return this->sendLen;
}

void FlyClient::setSendLen(size_t sentLen) {
    this->sendLen = sentLen;
}

void FlyClient::addSendLen(size_t sentLen) {
    this->sendLen += sentLen;
}

AbstractFlyDB *FlyClient::getFlyDB() const {
    return this->flyDB;
}

void FlyClient::setFlyDB(AbstractFlyDB *flyDB) {
    this->flyDB = flyDB;
}

const char *FlyClient::getReplid() const {
    return this->replid;
}

void FlyClient::setReplid(const char* replid) {
    memcpy(this->replid, replid, sizeof(replid));
}

int64_t FlyClient::getReploff() const {
    return this->reploff;
}

void FlyClient::setReploff(uint64_t reploff) {
    this->reploff = reploff;
}

int64_t FlyClient::getReadReploff() const {
    return this->readReploff;
}

void FlyClient::setReadReploff(int64_t readReploff) {
    this->readReploff = readReploff;
}
