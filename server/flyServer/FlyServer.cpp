//
// Created by 赵立伟 on 2018/9/18.
//

#include <iostream>
#include <syslog.h>
#include "FlyServer.h"
#include "../commandTable/CommandTable.h"
#include "../../def.h"
#include "../atomic/AtomicHandler.h"
#include "../net/NetDef.h"
#include "../utils/MiscTool.h"
#include "../net/NetHandler.h"
#include "../log/FileLogHandler.h"
#include "../flyClient/ClientDef.h"
#include "../fdb/FDBHandler.h"
#include "../db/FlyDB.h"
#include "../log/FileLogFactory.h"
#include "../db/FlyDBFactory.h"

FlyServer::FlyServer(const AbstractCoordinator *coordinator) {
    this->coordinator = coordinator;
    
    // fly db factory
    this->flyDBFactory = new FlyDBFactory(this->coordinator);

    // init db array
    for (int i = 0; i < DB_NUM; i++) {
        this->dbArray[i] = this->flyDBFactory->getFlyDB();
        if (NULL == this->dbArray.at(i)) {
            std::cout << "error to create FlyDB[" << i << "]" << std::endl;
            exit(1);
        }
    }

    // init command table
    this->commandTable = new CommandTable(coordinator);

    // 设置最大客户端数量
    setMaxClientLimit();

    // serverCron运行频率
    this->hz = CONFIG_CRON_HZ;
    this->neterr = new char[NET_ERR_LEN];

    // 拒绝连接次数设置为0
    this->statRejectedConn = 0;
    pthread_mutex_init(&this->nextClientIdMutex, NULL);

    // 当前时间
    this->nowt = time(NULL);
    this->clientMaxQuerybufLen = PROTO_MAX_QUERYBUF_LEN;
}

FlyServer::~FlyServer() {
    for (int i = 0; i < DB_NUM; i++) {
        delete this->dbArray.at(i);
    }
    delete this->commandTable;
    delete[] this->neterr;
    closelog();
}

void FlyServer::loadFromConfig(ConfigCache *configCache) {
    this->bindAddr = configCache->getBindAddrs();
    this->unixsocket = configCache->getUnixsocket();
    this->unixsocketperm = configCache->getUnixsocketperm();
    this->tcpKeepAlive = configCache->getTcpKeepAlive();
    this->port = configCache->getPort();
}

void FlyServer::init(ConfigCache *configCache) {
    // 从configCache获取参数
    this->loadFromConfig(configCache);

    // 时间循环处理器
    this->coordinator->getEventLoop()->createTimeEvent(
            1, serverCron, NULL, NULL);

    // 打开监听socket，用于监听用户命令
    this->listenToPort();

    // 打开Unix domain socket
    if (NULL != this->unixsocket) {
        unlink(this->unixsocket);       // 如果存在，则删除unixsocket文件
        this->usfd = this->coordinator->getNetHandler()->unixServer(
                this->neterr,
                this->unixsocket,
                this->unixsocketperm,
                this->tcpBacklog);
        if (-1 == this->usfd) {
            std::cout << "Opening Unix Domain Socket: "
                         << this->neterr << std::endl;
            exit(1);
        }
        this->coordinator->getNetHandler()->setBlock(NULL, this->usfd, 0);
    }

    // 创建定时任务，用于创建客户端连接
    for (auto fd : this->ipfd) {
        if (-1 == this->coordinator->getEventLoop()->createFileEvent(
                fd, ES_READABLE, acceptTcpHandler, NULL)) {
            exit(1);
        }
    }

    this->logHandler = logFactory->getLogger();

    // 从fdb或者aof中加载数据
    loadDataFromDisk();

    // 信号处理
    this->setupSignalHandlers();

    return;
}

pid_t FlyServer::getPID() {
    return this->pid;
}

std::string FlyServer::getVersion() {
    return this->version;
}

int FlyServer::dealWithCommand(AbstractFlyClient *flyclient) {
    return this->commandTable->dealWithCommand(flyclient);
}

int FlyServer::getHz() const {
    return hz;
}

void FlyServer::setHz(int hz) {
    FlyServer::hz = hz;
}

char *FlyServer::getNeterr() const {
    return neterr;
}


time_t FlyServer::getNowt() const {
    return nowt;
}

void FlyServer::setNowt(time_t nowt) {
    FlyServer::nowt = nowt;
}

size_t FlyServer::getClientMaxQuerybufLen() const {
    return clientMaxQuerybufLen;
}

int64_t FlyServer::getStatNetInputBytes() const {
    return statNetInputBytes;
}

void FlyServer::addToStatNetInputBytes(int64_t size) {
    this->clientMaxQuerybufLen += size;
}

void FlyServer::closeListeningSockets(bool unlinkUnixSocket) {
    // 关闭所有监听连接socket
    int ipCount = this->ipfd.size();
    for (int i = 0; i < ipCount; i++) {
        close(this->ipfd[i]);
    }

    // 关闭unix socket file descriptor
    if (-1 != this->usfd) {
        close(this->usfd);
    }
    if (unlinkUnixSocket && NULL != this->unixsocket) {
        this->logHandler->logNotice("Removing the unix socket file.");
        unlink(this->unixsocket);
    }

    // todo cluster enabled
}

void FlyServer::setMaxClientLimit() {
    this->maxClients = CONFIG_DEFAULT_MAX_CLIENTS;
    int maxFiles = this->maxClients + CONFIG_MIN_RESERVED_FDS;
    rlimit limit;

    // 获取当前进程可打开的最大文件描述符
    if (getrlimit(RLIMIT_NOFILE, &limit) == -1) {
        // 如果获取失败, 按照进程中最大文件数量为1024计算(内核默认1024),
        // 重置maxClients
        this->maxClients = 1024 - CONFIG_MIN_RESERVED_FDS;
    } else {
        int softLimit = limit.rlim_cur;
        // 如果soft limit小于maxfiles, 则尝试增大soft limit, 并重置maxClients
        if (softLimit < maxFiles) {
            int curLimit = maxFiles;
            int decrStep = 16;

            // 逐步试探提高softlimit
            while (curLimit > softLimit) {
                limit.rlim_cur = curLimit;
                if (setrlimit(RLIMIT_NOFILE, &limit) != -1) {
                    break;
                }
                curLimit -= decrStep;
            }
            if (curLimit < softLimit) {
                curLimit = softLimit;
            }

            // 如果当前文件数量限制小于最小保留文件数，程序退出
            if (curLimit <= CONFIG_MIN_RESERVED_FDS) {
                exit(1);
            }

            this->maxClients = curLimit - CONFIG_MIN_RESERVED_FDS;
        }
    }
}

int FlyServer::listenToPort() {
    int fd;
    // try to bind all to IPV4 and IPV6
    if (0 == this->bindAddr.size()) {
        int success = 0;
        // try to set *(any address) to ipv6
        fd = this->coordinator->getNetHandler()->tcp6Server(
                this->neterr,
                this->port,
                NULL,
                this->tcpBacklog);
        if (fd != -1) {
            // set nonblock
            this->coordinator->getNetHandler()->setBlock(NULL, fd, 0);
            this->ipfd.push_back(fd);
            success++;
        }

        // try to set *(any address) to ipv4
        fd = this->coordinator->getNetHandler()->tcpServer(
                this->neterr,
                this->port,
                NULL,
                this->tcpBacklog);
        if (fd != -1) {
            // set nonblock
            this->coordinator->getNetHandler()->setBlock(NULL, fd, 0);
            this->ipfd.push_back(fd);
            success++;
        }

        if (0 == success) {
            return -1;
        }
    } else {
        for (auto addr : this->bindAddr) {
            // 如果是IPV6
            if (addr.find(":") != addr.npos) {
                fd = this->coordinator->getNetHandler()->tcp6Server(
                        this->neterr,
                        this->port,
                        addr.c_str(),
                        this->tcpBacklog);
            } else {
                fd = this->coordinator->getNetHandler()->tcpServer(
                        this->neterr,
                        this->port,
                        addr.c_str(),
                        this->tcpBacklog);
            }
            if (-1 == fd) {
                return -1;
            }
            this->coordinator->getNetHandler()->setBlock(NULL, fd, 0);
            this->ipfd.push_back(fd);
        }
    }

    return 1;
}


void FlyServer::loadDataFromDisk() {
    // 如果开启了AOF，则优先从AOF中加载持久化数据，否则从FDB中加载
    if (this->coordinator->getAofHandler()->IsStateOn()) {
        // load from append only fiile
    } else {
        // load from fdb
        FDBSaveInfo *fdbSaveInfo = new FDBSaveInfo();
        this->coordinator->getFdbHandler()->load(fdbSaveInfo);
        delete fdbSaveInfo;
    }

}

void FlyServer::freeClientsInAsyncFreeList() {
    for (auto client : this->clientsToClose) {
        deleteClient(client->getFd());
    }

    this->clientsToClose.clear();
}

AbstractFlyDB* FlyServer::getFlyDB(int dbnum) {
    if (dbnum >= this->dbArray.size()) {
        return NULL;
    }

    return this->dbArray[dbnum];
}

uint8_t FlyServer::getFlyDBCount() const {
    return this->dbArray.size();
}


AbstractFlyClient* FlyServer::createClient(int fd) {
    if (fd <= 0) {
        return NULL;
    }

    // 超过了客户端最大数量
    if (this->clients.size() >= this->maxClients) {
        this->statRejectedConn++;
        return NULL;
    }

    // create FlyClient
    AbstractFlyClient *flyClient = this->coordinator
            ->getFlyClientFactory()
            ->getFlyClient(fd, coordinator, this->getFlyDB(0));
    uint64_t clientId = 0;
    atomicGetIncr(this->nextClientId, clientId, 1);
    flyClient->setId(clientId);

    // 设置读socket，并为其创建相应的file event
    this->coordinator->getNetHandler()->setBlock(NULL, fd, 0);
    this->coordinator->getNetHandler()->setTcpNoDelay(NULL, fd, 1);
    if (this->tcpKeepAlive > 0) {
        this->coordinator->getNetHandler()->keepAlive(
                NULL, fd, this->tcpKeepAlive);
    }
    if (-1 == this->coordinator->getEventLoop()->createFileEvent(
            fd, ES_READABLE, readQueryFromClient, flyClient)) {
        delete flyClient;
        return NULL;
    }

    // 加入到clients队列中
    this->clients.push_back(flyClient);

    return flyClient;
}

int FlyServer::deleteClient(int fd) {
    std::list<AbstractFlyClient *>::iterator iter = this->clients.begin();
    for (iter; iter != this->clients.end(); iter++) {
        if (fd == (*iter)->getFd()) {
            // 删除file event
            this->coordinator->getEventLoop()->deleteFileEvent(
                    fd, ES_READABLE | ES_WRITABLE);

            // 在相应列表中删除
            this->deleteFromAsyncClose(fd);
            this->deleteFromPending(fd);

            // 删除FlyClient
            delete (*iter);
            this->clients.erase(iter);

            return 1;
        }
    }

    // 没有找到对应的FlyClient
    return -1;
}

void FlyServer::addToClientsPendingToWrite(AbstractFlyClient *flyClient) {
    this->clientsPendingWrite.push_back(flyClient);
}

int FlyServer::handleClientsWithPendingWrites() {
    if (0 == this->clientsPendingWrite.size()) {
        return 0;
    }

    std::list<AbstractFlyClient*>::iterator iter =
            this->clientsPendingWrite.begin();
    for (iter; iter != this->clientsPendingWrite.end(); iter++) {
        // 先清除标记，清空了该标记才回保证该客户端再次加入到clientsPendingWrite里；
        // 否则无法加入。也就无法处理其输出
        (*iter)->delFlag(~CLIENT_PENDING_WRITE);

        // 先直接发送，如果发送不完，再创建文件事件异步发送
        if (-1 == this->coordinator->getNetHandler()->writeToClient(
                this->coordinator, *iter, 0)) {
            continue;
        }

        // 异步发送
        if (!(*iter)->hasNoPending()
            && -1 == this->coordinator->getEventLoop()->createFileEvent(
                    (*iter)->getFd(), ES_WRITABLE, sendReplyToClient, *iter)) {
            freeClientAsync(*iter);
        }
    }

    int pendingCount = this->clientsPendingWrite.size();
    this->clientsPendingWrite.clear();
    return pendingCount;
}

void FlyServer::freeClientAsync(AbstractFlyClient *flyClient) {
    if (flyClient->getFlags() & CLIENT_CLOSE_ASAP) {
        return;
    }

    flyClient->setFlags(CLIENT_CLOSE_ASAP);
    this->clientsToClose.push_back(flyClient);
}

void FlyServer::deleteFromPending(int fd) {
    std::list<AbstractFlyClient*>::iterator iter =
            this->clientsPendingWrite.begin();
    for (iter; iter != this->clientsPendingWrite.end(); iter++) {
        if ((*iter)->getFd() == fd) {
            this->clientsPendingWrite.erase(iter);
            return;
        }
    }
}

void FlyServer::deleteFromAsyncClose(int fd) {
    std::list<AbstractFlyClient*>::iterator iter = this->clientsToClose.begin();
    for (iter; iter != this->clientsToClose.end(); iter++) {
        if ((*iter)->getFd() == fd) {
            this->clientsToClose.erase(iter);
            return;
        }
    }
}

int FlyServer::getMaxClients() const {
    return maxClients;
}

void FlyServer::setupSignalHandlers() {
    signal(SIGTERM, sigShutDownHandlers);
    signal(SIGINT, sigShutDownHandlers);
}

bool FlyServer::isShutdownASAP() const {
    return this->shutdownASAP;
}

void FlyServer::setShutdownASAP(bool shutdownASAP) {
    this->shutdownASAP = shutdownASAP;
}

void sigShutDownHandlers(int sig) {
    extern AbstractCoordinator *coordinator;

    if (coordinator->getFlyServer()->isShutdownASAP()
        && SIGINT == sig) {
        coordinator->getFdbHandler()->deleteTempFile(getpid());
        exit(1);
    }

    coordinator->getFlyServer()->setShutdownASAP(true);
}

int serverCron(const AbstractCoordinator *coordinator,
               uint64_t id,
               void *clientData) {
    AbstractFlyServer *flyServer = coordinator->getFlyServer();

    // 设置当前时间
    flyServer->setNowt(time(NULL));

    // 释放所有异步删除的clients
    flyServer->freeClientsInAsyncFreeList();

    if (coordinator->getFlyServer()->isShutdownASAP()) {
        coordinator->getFdbHandler()->save();
    }

    // 如果有fdb或者aof子进程存在的话
    if (coordinator->getAofHandler()->haveChildPid()
        || coordinator->getFdbHandler()->haveChildPid()) {
        int statloc;
        pid_t pid = -1;

        if ((pid = wait3(&statloc, WNOHANG, NULL)) != 0) {
            int exitCode = WEXITSTATUS(statloc);
            int bySignal = 0;
            if (WIFSIGNALED(statloc)) {
                bySignal = WTERMSIG(statloc);
            }

            if (-1 == pid) {
                coordinator->getLogHandler()->logWarning(
                        "wait3() returned an error: %s. "
                        "rdb_child_pid = %d, aof_child_pid = %d",
                        strerror(errno),
                        coordinator->getFdbHandler()->getChildPid(),
                        coordinator->getAofHandler()->getChildPid());
            } else if (coordinator->getFdbHandler()->getChildPid() == pid) {
                coordinator->getFdbHandler()->backgroundSaveDone(
                        exitCode, bySignal);
            } else if (coordinator->getAofHandler()->getChildPid() == pid) {
                // todo:
            } else {
                coordinator->getLogHandler()->logWarning(
                        "Warning, detected child with unmatched pid: %ld", pid);

            }

            coordinator->getPipe()->closeAll();
        }
    } else {
        /**
         * 查看是否达到fdb执行条件，即：
         * saveParam->dirty>changes并且save sap < saveParam->seconds
         **/
        int count = coordinator->getFdbHandler()->getSaveParamsCount();
        for (int i = 0; i < count; i++) {
            const saveParam* saveParam =
                    coordinator->getFdbHandler()->getSaveParam(i);
            if (coordinator->getFdbHandler()->getDirty() > saveParam->changes
                && coordinator->getFdbHandler()->
                    lastSaveTimeGapGreaterThan(saveParam->seconds)
                && coordinator->getFdbHandler()->canBgsaveNow()
                && 1 == coordinator->getFdbHandler()->getLastBgsaveStatus()) {
                coordinator->getFdbHandler()->backgroundSave();
            }
        }
    }

    // 处理被AOF延迟了的FDB操作
    if (!coordinator->getFdbHandler()->haveChildPid()
        && !coordinator->getAofHandler()->haveChildPid()
        && coordinator->getFdbHandler()->isBGSaveScheduled()
        && coordinator->getFdbHandler()->canBgsaveNow()
        && 1 == coordinator->getFdbHandler()->getLastBgsaveStatus()) {
        // fdb被成功执行了，则下次不再schedule
        if (1 == coordinator->getFdbHandler()->backgroundSave()) {
            coordinator->getFdbHandler()->setBGSaveScheduled(false);
        }
    }

    static int times = 0;
    std::cout << "serverCron is running " << ++times << " times!" << std::endl;

    return 1000 / flyServer->getHz();
}
