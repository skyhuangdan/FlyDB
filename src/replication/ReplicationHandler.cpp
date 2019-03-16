//
// Created by 赵立伟 on 2019/3/6.
//

#include <cassert>
#include <fcntl.h>
#include "ReplicationHandler.h"
#include "../flyClient/ClientDef.h"
#include "../io/StringFio.h"
#include "../net/NetHandler.h"

ReplicationHandler::ReplicationHandler(AbstractCoordinator *coordinator) {
    this->coordinator = coordinator;
    this->logHandler = logFactory->getLogger();
    this->stateAdapter = new ReplicationStateAdapter();
}

void ReplicationHandler::unsetMaster() {
    if (!this->haveMasterhost()) {
        return;
    }

    /** shift replication id */
    this->shiftReplicationId();

    /** 删除master并取消与master的replication握手 */
    this->cancelHandShake();

    /** 删除cached master */
    this->discardCachedMaster();

    /**
     * 用以通知所有的slave, 本机的replication id改变了，
     * slave会向本机发送psync，这样便可以重新建立连接
     **/
    this->disconnectWithSlaves();

    /** replication state */
    this->state = REPL_STATE_NONE;

    /**
     * 这里将slaveSelDB设置为1，为了在下次full resync时强加一个SELECT命令
     **/
    this->slaveSelDB = -1;
}

void ReplicationHandler::setMaster(std::string ip, int port) {
    /** 如果this->master是空，则代表本机曾经是主 */
    int wasMaster = this->masterhost.empty();
    this->masterhost = ip;
    this->masterport = port;

    /** 删除原先的master */
    if (NULL != this->master) {
        coordinator->getFlyServer()->freeClient(this->master);
        this->master = NULL;
    }

    /** 令所有slave重新连接 */
    this->disconnectWithSlaves();

    /** 取消与master的握手 */
    this->cancelHandShake();

    /** 缓存master */
    if (wasMaster) {
        this->cacheMasterUsingMyself();
    }

    this->state = REPL_STATE_CONNECT;
    this->masterDownSince = 0;
}

const std::string &ReplicationHandler::getMasterhost() const {
    return this->masterhost;
}

int ReplicationHandler::getMasterport() const {
    return this->masterport;
}

bool ReplicationHandler::haveMasterhost() const {
    return !this->masterhost.empty();
}

void ReplicationHandler::cron() {
    time_t nowt = coordinator->getFlyServer()->getNowt();

    /** 握手或者连接中，且超时 */
    if (this->haveMasterhost() &&
        (REPL_STATE_CONNECTING == this->state || this->inHandshakeState())
        && nowt - this->transferLastIO > this->timeout) {
        this->cancelHandShake();
    }

    /** 正在处于fdb文件传输阶段并超时 */
    if (this->haveMasterhost()
        && REPL_STATE_TRANSFER == this->state
        && nowt - this->transferLastIO > this->timeout) {
        this->cancelHandShake();
    }

    /** 已连接阶段，并超时*/
    if (this->haveMasterhost()
        && REPL_STATE_CONNECTED == this->state
        && nowt - this->lastInteraction > this->timeout) {
        coordinator->getFlyServer()->freeClient(this->master);
    }

    /** 等待连接状态 */
    if (REPL_STATE_CONNECT == this->state) {
        logHandler->logNotice("Connecting to MASTER %s:%d",
                              masterhost.c_str(),
                              masterport);
        if (1 == this->connectWithMaster()) {
            logHandler->logNotice("MASTER <--> SLAVE sync started!");
        }
    }

    /** 心跳检测：周期性（每秒一次）回复master */
    if (!this->masterhost.empty() && NULL != this->master) {
        sendAck();
    }

    // todo:
}


void ReplicationHandler::syncWithMasterStatic(
        const AbstractCoordinator *coorinator,
        int fd,
        std::shared_ptr<AbstractFlyClient> flyClient,
        int mask) {
    coorinator->getReplicationHandler()->syncWithMaster(fd, flyClient, mask);
}

void ReplicationHandler::syncWithMaster(
        int fd,                     /** 这里的fd就是this->transferSocket */
        std::shared_ptr<AbstractFlyClient> flyClient,
        int mask) {
    /** no active replication state */
    if (REPL_STATE_NONE == this->state) {
        close(fd);
        return;
    }

    /** 检查一遍socket的状态，一直处于sync状态，有可能socket已经出现了error */
    int sockerr;
    socklen_t errlen = sizeof(sockerr);
    if (-1 == getsockopt(fd, SOL_SOCKET, SO_ERROR, &sockerr, &errlen)) {
        sockerr = errno;
    }

    /** 调用相应的命令处理函数进行处理 */
    if (sockerr != 0 || -1 == this->stateAdapter->processState(this->state)) {
        logHandler->logWarning(
                "Something is wrong with this socket for SYNC: %s",
                strerror(sockerr));
        coordinator->getEventLoop()->deleteFileEvent(fd, ES_READABLE | ES_WRITABLE);
        close(fd);
        this->transferSocket = -1;
        this->state = REPL_STATE_CONNECT;
        return;
    }
}

int ReplicationHandler::connectingStateProcess() {
    this->logHandler->logNotice("Non blocking connect for SYNC fired the event.");

    /**
     * 删除写文件事件，只保留读文件事件，便于接收接下来的PONG回复
     * 由于这里删除了写文件时间，此时只有读事件可以被触发(对应各种recv函数)，因此在下面的状态中，
     * 当recv之后需要同时发送下一个阶段需要发送的数据，否则的话，发送状态将无法触发
     **/
    coordinator->getEventLoop()->deleteFileEvent(this->transferSocket, ES_WRITABLE);

    /** send ping command */
    if (!sendSynchronousCommand(this->transferSocket, "PING")) {
        return -1;
    }

    /** 设置状态为等待接收PONG */
    this->state = REPL_STATE_RECEIVE_PONG;
    return 1;
}

int ReplicationHandler::recvPongStateProcess() {
    std::string res = recvSynchronousCommand(this->transferSocket, NULL);
    /** 这里接收到的命令回复只有两种，一种是'+PONG'，这表示正常。或者是'-NOAUTH'，表示失败 */
    if ('+' == res.at(0)) {
        this->logHandler->logNotice("Master replied to PING, replication can continue...");
    } else {
        this->logHandler->logWarning("Error reply to PING from master: '%s", res.c_str());
        return -1;
    }

    /** 如果存在鉴权信息，则发送鉴权，否则直接跳过 */
    if (!masterAuth.empty()) {
        if (!sendSynchronousCommand(
                this->transferSocket, "AUTH", this->masterAuth.c_str(), this->masterUser.c_str(), NULL)) {
            return -1;
        }
        this->state = REPL_STATE_RECEIVE_AUTH;
    } else {
        this->state = REPL_STATE_SEND_PORT;
    }

    return 1;
}

int ReplicationHandler::recvAuthStateProcess() {
    std::string res = recvSynchronousCommand(this->transferSocket, NULL);
    /** 如果接收到第一个字符是'-'，代表鉴权错误, 鉴权错误则不能再继续了 */
    if ('-' == res.at(0)) {
        this->logHandler->logWarning("Unable AUTH to master: %s", res.c_str());
        return -1;
    }

    AbstractFlyServer *flyServer = coordinator->getFlyServer();
    int port = this->slaveAnnouncePort ? this->slaveAnnouncePort : flyServer->getPort();
    /** send REPLCONF listening-port [port] */
    if (!sendSynchronousCommand(this->transferSocket,
                                       "REPLCONF",
                                       "listening-port",
                                       std::to_string(port).c_str(),
                                       NULL)) {
        return -1;
    }

    this->state = REPL_STATE_RECEIVE_PORT;
    return 1;
}

int ReplicationHandler::recvPortStateProcess() {
    std::string res = recvSynchronousCommand(this->transferSocket, NULL);
    /** 这时发生的错误可以容忍，所以只是打印错误日志，状态机继续 */
    if ('-' == res.at(0)) {
        logHandler->logWarning("Master does not understand REPLCONF listening-port: %s", res.c_str());
    }

    /** 如果没有slaveAnnounceIp, 则跳过发送ip阶段 */
    if (this->slaveAnnounceIP.empty()) {
        this->state = REPL_STATE_SEND_CAPA;
    } else {
        /** send REPLCONF ip-address [ip] */
        if (!sendSynchronousCommand(this->transferSocket, "REPLCONF",
                                           "ip-address", this->slaveAnnounceIP.c_str(), NULL)) {
            return -1;
        }
        this->state = REPL_STATE_RECEIVE_IP;
    }

    return 1;
}

int ReplicationHandler::recvIPStateProcess() {
    std::string res = recvSynchronousCommand(this->transferSocket, NULL);
    /** 这时发生的错误可以容忍，所以只是打印错误日志，状态机继续 */
    if ('-' == res.at(0)) {
        logHandler->logWarning("Master does not understand REPLCONF ip-address: %s", res.c_str());
    }

    /**
     * 通知master，当前服务器所支持的能力：
     *  1.eof代表支EOF格式的FDB文件
     *  2.PSYNC2代表支持PSYNC v2，因此可以识别 +CONTINUE <new repl ID>
     **/
    if (!sendSynchronousCommand(this->transferSocket, "REPLCONF", "capa", "eof", "capa", "psync2", NULL)) {
        return -1;
    }

    this->state = REPL_STATE_RECEIVE_CAPA;
    return 1;
}

int ReplicationHandler::recvCAPAStateProcess() {
    std::string res = recvSynchronousCommand(this->transferSocket, NULL);
    /** 这时发生的错误可以容忍，所以只是打印错误日志，状态机继续 */
    if ('-' == res.at(0)) {
        logHandler->logWarning("Master does not understand REPLCONF capa: %s", res.c_str());
    }

    if (PSYNC_WRITE_ERROR == slaveTrySendPartialResynchronization(this->transferSocket)) {
        return -1;
    }

    this->state = REPL_STATE_RECEIVE_PSYNC;
    return 1;
}

int ReplicationHandler::recvPsyncStateProcess() {
    int result = slaveTryRecvPartialResynchronization(this->transferSocket);
    switch (result) {
        case PSYNC_TRY_LATER:
            return -1;
        case PSYNC_WAIT_REPLY:
            return 0;
        case PSYNC_CONTINUE:
            logHandler->logNotice("MASTER <-> REPLICA sync: Master accepted a Partial Resynchronization.");
            return 1;
        case PSYNC_FULLRESYNC:
            this->disconnectWithSlaves();
            /** 全量更新，不需要backlog */
            this->freeReplicationBacklog();
            break;
        default:
            return -1;
    }

    int maxTries = 5;
    int tempfd = -1;
    char tempfile[256];
    while (maxTries--) {
        snprintf(tempfile, sizeof(tempfile), "temp-%d.%ld.fdb", coordinator->getFlyServer()->getNowt(), getpid());
        if (-1 != (tempfd = open(tempfile, O_CREAT | O_WRONLY | O_EXCL, 0644))) {
            break;
        }

        /** 创建文件失败 */
        sleep(1);
    }

    /** 创建文件事件用于同步fdb文件 */
    if (coordinator->getEventLoop()->createFileEvent(this->transferSocket, ES_READABLE, NULL, NULL)) {
        logHandler->logWarning("Error to create readable event for sync: %s", strerror(errno));
        return -1;
    }

    this->initTransfer(tempfd, tempfile);
    this->state = REPL_STATE_TRANSFER;
}

void ReplicationHandler::sendAck() {
    std::shared_ptr<AbstractFlyClient> flyClient = this->master;
    if (NULL == flyClient) {
        return;
    }

    flyClient->addFlag(CLIENT_MASTER_FORCE_REPLY);
    flyClient->addReplyBulkCount(3);
    flyClient->addReplyBulkString("REPLCONF");
    flyClient->addReplyBulkString("ACK");
    flyClient->addReplyBulkString(std::to_string(this->offset));
}

std::string ReplicationHandler::recvSynchronousCommand(int fd, ...) {
    char buf[256];
    if (-1 == coordinator->getNetHandler()->syncReadLine(fd, buf, sizeof(buf), this->syncioTimeout * 1000)) {
        char temp[100];
        snprintf(temp, sizeof(temp), "-Reading from master: %s", strerror(errno));
        return temp;
    }

    this->transferLastIO = coordinator->getFlyServer()->getNowt();
    return buf;
}

bool ReplicationHandler::sendSynchronousCommand(int fd, ...) {
    std::string cmd;

    va_list ap;
    va_start(ap, fd);
    char* arg;
    while (1) {
        /** 获取下一个参数 */
        arg = va_arg(ap, char*);
        if (NULL == arg) {
            break;
        }

        /** 添加到写入缓冲里 */
        char buf[100];
        snprintf(buf, sizeof(buf), "$%zu\r\n%s", strlen(arg), arg);
        cmd += buf;
    }
    va_end(ap);

    if (-1 == coordinator->getNetHandler()->syncWrite(fd, cmd, this->syncioTimeout * 1000)) {
        logHandler->logWarning("Error to syncWrite to master: %s", strerror(errno));
        return false;
    }

    return true;
}

PsyncResult ReplicationHandler::slaveTrySendPartialResynchronization(int fd) {
    char psyncOffset[32];
    const char *psyncReplid;

    /** 在这里暂时设置master offset为-1，随后在昨晚FULL SYNC后会对这个值重置 */
    this->masterInitOffset = -1;
    if (NULL != this->cachedMaster) {
        psyncReplid = this->cachedMaster->getReplid();
        snprintf(psyncOffset, sizeof(psyncOffset), "%lld", this->cachedMaster->getReploff());
    } else {
        psyncReplid = "?";
        memcpy(psyncOffset, "-1", 3);
    }

    bool res = sendSynchronousCommand(fd, "PSYNC", psyncReplid, psyncOffset, NULL);
    if (!res) {
        /** 发送错误，删除读取文件事件 */
        coordinator->getEventLoop()->deleteFileEvent(fd, ES_READABLE);
        logHandler->logWarning("Unable to send PSYNC to master");
        return PSYNC_WRITE_ERROR;
    }

    return PSYNC_WAIT_REPLY;
}

PsyncResult ReplicationHandler::slaveTryRecvPartialResynchronization(int fd) {
    /** 获取reply */
    std::string reply = recvSynchronousCommand(fd, NULL);
    if (reply.length()) {
        return PSYNC_WAIT_REPLY;
    }

    /** 删除读取文件事件 */
    coordinator->getEventLoop()->deleteFileEvent(fd, ES_READABLE);

    if (!strncmp(reply.c_str(), "+FULLRESYNC", 11)) { /** 全量读取 */
        this->dealWithFullResyncReply(reply);
        return PSYNC_FULLRESYNC;
    } else if (!strncmp(reply.c_str(), "+CONTIINUE", 9)) { /** 部分同步 */
        this->dealWithContinueReply(fd, reply);
        return PSYNC_CONTINUE;
    } else if (!strncmp(reply.c_str(), "-NOMASTERLINK", 13)
               || !strncmp(reply.c_str(), "-LOADING", 8)) {
        logHandler->logNotice("Master is unable to PSYNC, but should be in the future: %s", reply.c_str());
        return PSYNC_TRY_LATER;
    } else {
        logHandler->logWarning("Unexpected reply to PSYNC from master: %s", reply.c_str());
    }

    /** 释放cached master */
    this->discardCachedMaster();
    return PSYNC_NOT_SUPPORTED;
}

void ReplicationHandler::dealWithFullResyncReply(std::string reply) {
    /** 截取replid和offset */
    std::string replid;
    std::string offset;
    size_t idpos = reply.find(" ");
    size_t offsetpos = -1;
    if (-1 != idpos) {
        replid = reply.substr(0, idpos - 1);
        offsetpos = reply.find(" ", idpos);
        if (-1 != offsetpos) {
            offset = reply.substr(idpos, offsetpos - 1);
        }
    }

    /** 设置replid和masterInitOffset */
    if (-1 == idpos || -1 == offsetpos || (offsetpos - idpos - 1) != CONFIG_RUN_ID_SIZE) {
        memset(this->replid, 0, CONFIG_RUN_ID_SIZE + 1);
        logHandler->logWarning("Master replied with wrong +FULLRESYNC syntax.");
    } else {
        memcpy(this->replid, replid.c_str(), replid.length());
        this->replid[CONFIG_RUN_ID_SIZE] = '\0';
        this->masterInitOffset = atoll(offset.c_str());
        logHandler->logNotice("Full resync from master: %s:%lld", this->replid, this->masterInitOffset);
    }

    this->discardCachedMaster();
}

void ReplicationHandler::dealWithContinueReply(int fd, std::string reply) {
    logHandler->logNotice("Successful PSYNC with master.");

    size_t headpos = 10;
    size_t tailpos = 9;
    while ('\r' != reply.at(tailpos) && '\n' != reply.at(tailpos) && tailpos < reply.length()) {
        tailpos++;
    }

    if (CONFIG_RUN_ID_SIZE == tailpos - headpos) {
        /** 截取replication id */
        char replid[CONFIG_RUN_ID_SIZE + 1];
        std::string replidstr = reply.substr(headpos, tailpos);
        memcpy(replid, replidstr.c_str(), CONFIG_RUN_ID_SIZE);
        replid[CONFIG_RUN_ID_SIZE + 1] = '\0';

        /** 如果当前获取的replid和cachedMaster中保存的replid不同 */
        if (strcmp(replid, this->cachedMaster->getReplid())) {
            logHandler->logWarning("Master replication ID changed to %s", replid);

            /** (cacheMaster->replid) --> replid2 */
            memcpy(this->replid2, this->cachedMaster->getReplid(), sizeof(this->replid2));
            this->secondReplOffset = this->masterReplOffset + 1;

            /** replid --> (this->replid) */
            memcpy(this->replid, replid, sizeof(replid));

            /** replid --> (cachedMaster->replid) */
            cachedMaster->setReplid(replid);

            /** 重连接所有的slave */
            this->disconnectWithSlaves();
        }
    }
    this->resurrectCachedMaster(fd);

    /** 如果当前环形缓冲区为空，则创建一个 */
    if (NULL == this->backlog) {
        this->createReplicationBacklog();
    }
}

int ReplicationHandler::cancelHandShake() {
    if (REPL_STATE_TRANSFER == this->state) {
        /** 如果处于正在传输fdb文件的阶段 */
        this->abortSyncTransfer();
        this->state = REPL_STATE_CONNECT;
    } else if (REPL_STATE_CONNECTING == this->state
               || this->inHandshakeState()) {
        /** 如果正处于连接或者握手阶段 */
        this->disconnectWithMaster();
        this->state = REPL_STATE_CONNECT;
    } else {
        /** 否则，处于完成或者NONE状态 */
        return 0;
    }

    return 1;
}

/**
 * 当一个服务器由master切换成slave时调用
 *  为了创建一个cached master，将其用于在晋升为master时与slave做PSYNC用
 * */
void ReplicationHandler::cacheMasterUsingMyself() {
    AbstractFlyServer *flyServer = coordinator->getFlyServer();

    flyServer->unlinkClient(this->master);
    this->master = NULL;
    this->cachedMaster = this->master;
}

int ReplicationHandler::connectWithMaster() {
    AbstractFlyServer* flyServer = coordinator->getFlyServer();

    /** 创建与master的连接 */
    int fd = coordinator->getNetHandler()->tcpNonBlockBestEffortBindConnect(
            NULL,
            this->masterhost.c_str(),
            this->masterport,
            flyServer->getFirstBindAddr()->c_str());
    if (-1 == fd) {
        logHandler->logWarning("Unable to connect to MASTER: %S",
                               strerror(errno));
        return -1;
    }

    /** 创建网络事件, 用于和master通信 */
    if (-1 == coordinator->getEventLoop()->createFileEvent(
            fd,
            ES_READABLE | ES_WRITABLE,
            syncWithMasterStatic,
            NULL)) {
        close(fd);
        logHandler->logWarning("Can`t create readable event for SYNC");
        return -1;
    }

    this->transferLastIO = flyServer->getNowt();
    this->transferSocket = fd;
    this->state = REPL_STATE_CONNECTING;
    return 1;
}

bool ReplicationHandler::abortSyncTransfer() {
    assert(REPL_STATE_TRANSFER == this->state);
    this->disconnectWithMaster();
    close(this->transferSocket);
    unlink(this->transferTempFile.c_str());
    this->transferTempFile.clear();
}

bool ReplicationHandler::inHandshakeState() {
    return this->state >= REPL_STATE_RECEIVE_PONG &&
           this->state <= REPL_STATE_RECEIVE_PSYNC;
}

void ReplicationHandler::disconnectWithMaster() {
    int fd = this->transferSocket;
    coordinator->getEventLoop()->deleteFileEvent(fd, ES_READABLE | ES_WRITABLE);
    close(fd);
    this->transferSocket = -1;
}

void ReplicationHandler::disconnectWithSlaves() {
    this->slaves.clear();
}

void ReplicationHandler::discardCachedMaster() {
    logHandler->logWarning("Discarding previously cached master state");
    this->cachedMaster = NULL;
}

/** 将cached master切换到current master */
void ReplicationHandler::resurrectCachedMaster(int fd) {
    this->master = this->cachedMaster;
    this->cachedMaster = NULL;
    this->master->setFd(fd);
    this->master->delFlag(CLIENT_CLOSE_AFTER_REPLY | CLIENT_CLOSE_ASAP);
    this->master->setAuthentiated(1);
    this->master->setLastInteractionTime(coordinator->getFlyServer()->getNowt());
    this->state = REPL_STATE_CONNECTED;
    this->masterDownSince = 0;

    /** add master to client list */
    coordinator->getFlyServer()->linkClient(this->master);
    if (-1 == this->coordinator->getEventLoop()->createFileEvent(
            fd, ES_READABLE, readQueryFromClient, this->master)) {
        coordinator->getFlyServer()->freeClientAsync(this->master);
    }

    /** 当回复缓冲区中有数据的时候，需要创建回复文件事件 */
    if (!this->master->hasNoPending()) {
        if (-1 == this->coordinator->getEventLoop()->createFileEvent(
                fd, ES_WRITABLE, sendReplyToClient, this->master)) {
            coordinator->getFlyServer()->freeClientAsync(this->master);
        }
    }
}

/**
 * 本机由从切换到主，replication id切换：
 *  1.将replication id/offset存入replication id2/offset2
 *  2.secondReplOffset = masterReplOffset + 1，这样在切换成slave时，slave取其已经拥有的字节的下一个字节
 *  3.重新生成replication id
 **/
void ReplicationHandler::shiftReplicationId() {
    /** replid-->replid2，将replid2保存其前任master的id */
    memcpy(this->replid2, this->replid, sizeof(this->replid));
    this->secondReplOffset = this->masterReplOffset + 1;
    this->randomReplicationId();
}

void ReplicationHandler::randomReplicationId() {
    uint8_t len = sizeof(this->replid);
    miscTool->getRandomHexChars(this->replid, len);
    this->replid[len] = '\0';
}

void ReplicationHandler::createReplicationBacklog() {
    if(NULL != this->backlog) {
        return;
    }

    this->backlog = (char*)malloc(this->backlogSize);
    this->backlogHistlen = 0;
    this->backlogIndex = 0;
    this->backlogOff = masterReplOffset + 1;
}

void ReplicationHandler::freeReplicationBacklog() {
    assert(0 == this->slaves.size());
    free(this->backlog);
    this->backlog = NULL;
}

void ReplicationHandler::initTransfer(int fd, char *fileName) {
    this->transferSize = -1;
    this->transferRead = 0;
    this->transferLastFsyncOff = 0;
    this->transferfd = fd;
    this->transferLastIO = coordinator->getFlyServer()->getNowt();
    this->transferTempFile = strdup(fileName);
}
