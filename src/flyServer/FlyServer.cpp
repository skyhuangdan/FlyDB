//
// Created by 赵立伟 on 2018/9/18.
//

#include <iostream>
#include <syslog.h>
#include <signal.h>
#include "FlyServer.h"
#include "../commandTable/CommandTable.h"
#include "../def.h"
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
#include "../dataStructure/dict/Dict.cpp"
#include "../bio/BIODef.h"
#include "../db/FlyDBDef.h"

#define runWithPeriod(flyServer, period) if(meetPeriod(flyServer, period))

/**
 * 当aof或者fdb子进程进行持久化的时候，可以设置canResize = true,
 * 不允许进行resize操作(除非在expand扩容时且ht.used > ht.size * NEED_FORCE_REHASH_RATIO)，
 * 这样可以减少内存搬移，以减少内存压力。具体可以搜索canResize，查看其使用场景
 */
bool canResize = true;

/** 共享multi bulk len字段，其格式为: "*<value>\r\n" */
std::shared_ptr<FlyObj> mbulkHeader[OBJ_SHARED_BULKHDR_LEN];

FlyServer::FlyServer(const AbstractCoordinator *coordinator) {
    this->coordinator = coordinator;
    
    // fly db factory
    this->flyDBFactory = new FlyDBFactory(this->coordinator);

    // init db array
    for (uint8_t i = 0; i < DB_NUM; i++) {
        this->dbArray[i] = this->flyDBFactory->getFlyDB(i);
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

    /** 创建共享FlyObj */
    this->createSharedObjects();

    return;
}

pid_t FlyServer::getPID() {
    return this->pid;
}

std::string FlyServer::getVersion() {
    return this->version;
}

int FlyServer::dealWithCommand(std::shared_ptr<AbstractFlyClient> flyclient) {
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
        this->coordinator->getAofHandler()->load();
    } else {
        // load from fdb
        FDBSaveInfo *fdbSaveInfo = new FDBSaveInfo();
        this->coordinator->getFdbHandler()->load(fdbSaveInfo);
        delete fdbSaveInfo;
    }

}

void FlyServer::freeClientsInAsyncFreeList() {
    for (auto client : this->clientsToClose) {
        if (NULL == client) {
            continue;
        }
        freeClient(client);
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

void FlyServer::updateDictResizePolicy() {
    if (coordinator->getAofHandler()->haveChildPid()
        || coordinator->getFdbHandler()->haveChildPid()) {
        canResize = false;
    }
    canResize = true;
}

void FlyServer::startToLoad() {
    this->loading = true;
    this->loadingStartTime = time(NULL);
}

void FlyServer::stopLoad() {
    this->loading = false;
}

bool FlyServer::isLoading() const {
    return this->loading;
}

/**
 * type:
 *  ACTIVE_EXPIRE_CYCLE_FAST：每次运行时间不超过EXPIRE_FAST_CYCLE_DURATION，并且下次开始
 *                            距离上次运行完时间间隔必须大于EXPIRE_FAST_CYCLE_DURATION
 *  ACTIVE_EXPIRE_CYCLE_SLOW: 运行时间是servercron运行hz的百分比时间，
 *                            百分比由ACTIVE_EXPIRE_CYCLE_SLOW_TIME_PERC指定
 **/
void FlyServer::activeExpireCycle(int type) {
    uint64_t start = miscTool->ustime();
    uint64_t timelimit = 0;
    uint8_t dbsPerCall = CRON_DBS_PER_CALL;

    if (ACTIVE_EXPIRE_CYCLE_FAST == type) {
        /** 如果不是上次超时，则不运行ACTIVE_EXPIRE_CYCLE_FAST，直接返回 */
        if (!this->timelimitExit) {
            return;
        }

        /**
         * 如果当前时间 < 上次运行时间+2*EXPIRE_FAST_CYCLE_DURATION
         * 2*EXPIRE_FAST_CYCLE_DURATION = 1*运行时间+1*运行间隔
         **/
        if (start < this->lastFastCycle + 2*ACTIVE_EXPIRE_CYCLE_FAST_DURATION) {
            return;
        }
        this->lastFastCycle = start;
        timelimit = ACTIVE_EXPIRE_CYCLE_FAST_DURATION;
    } else {
        timelimit = (uint64_t)(
                (1000000/this->hz) * (ACTIVE_EXPIRE_CYCLE_SLOW_TIME_PERC/100));
        if (timelimit <= 0) {
            timelimit = 1;
        }
    }

    /** 最大遍历的db数量不超过当前db的总数量 */
    uint8_t dbCount = this->getFlyDBCount();
    if (dbsPerCall > dbCount) {
        dbsPerCall = dbCount;
    }

    /** 遍历db */
    for (uint8_t i = 0; i < dbsPerCall; i++) {
        AbstractFlyDB *flyDB = this->getFlyDB(this->currentExpireDB);
        if (NULL == flyDB) {
            continue;
        }

        this->currentExpireDB =
                (this->currentExpireDB + 1) % this->getFlyDBCount();
        if (flyDB->activeExpireCycle(start, timelimit)) {
            this->timelimitExit = true;
            return;
        }
    }
}

void FlyServer::tryResizeDB() {
    uint8_t dbsPerCall = CRON_DBS_PER_CALL > this->getFlyDBCount()
            ? getFlyDBCount() : CRON_DBS_PER_CALL;
    for (uint8_t i = 0; i < dbsPerCall; i++) {
        AbstractFlyDB *flyDB = getFlyDB(this->currentShrinkDB);
        this->currentShrinkDB++;
        if (NULL == flyDB) {
            continue;
        }
        flyDB->tryResizeDB();
    }
}

std::shared_ptr<AbstractFlyClient> FlyServer::createClient(int fd) {
    if (fd <= 0) {
        return NULL;
    }

    // 超过了客户端最大数量
    if (this->clients.size() >= this->maxClients) {
        this->statRejectedConn++;
        return NULL;
    }

    // create FlyClient
    std::shared_ptr<AbstractFlyClient> flyClient = this->coordinator
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
        return NULL;
    }

    // 加入到clients队列中
    this->clients.push_back(flyClient);

    return flyClient;
}

int FlyServer::freeClient(std::shared_ptr<AbstractFlyClient> flyClient) {
    /** 将其从global list中删除*/
    this->unlinkClient(flyClient);

    // 在相应列表中删除
    this->deleteFromAsyncClose(flyClient->getFd());
}

void FlyServer::addToClientsPendingToWrite(
        std::shared_ptr<AbstractFlyClient> flyClient) {
    this->clientsPendingWrite.push_back(flyClient);
}

uint64_t FlyServer::addDirty(uint64_t count) {
    this->dirty += count;
    return this->dirty;
}

int FlyServer::handleClientsWithPendingWrites() {
    if (0 == this->clientsPendingWrite.size()) {
        return 0;
    }

    std::list<std::shared_ptr<AbstractFlyClient>>::iterator iter =
            this->clientsPendingWrite.begin();
    for (; iter != this->clientsPendingWrite.end(); iter++) {
        // 先清除标记，清空了该标记才回保证该客户端再次加入到clientsPendingWrite里；
        // 否则无法加入。也就无法处理其输出
        (*iter)->delFlag(CLIENT_PENDING_WRITE);

        // 先直接发送，如果发送不完，再创建文件事件异步发送
        if (-1 == this->coordinator->getNetHandler()->writeToClient(
                this->coordinator, *iter, 0)) {
            continue;
        }

        // 异步发送
        if (!(*iter)->hasNoPending()) {
            if (-1 == this->coordinator->getEventLoop()->createFileEvent(
                    (*iter)->getFd(),
                    ES_WRITABLE,
                    sendReplyToClient,
                    *iter)) {
                freeClientAsync(*iter);
            }

        }
    }

    int pendingCount = this->clientsPendingWrite.size();
    this->clientsPendingWrite.clear();
    return pendingCount;
}

void FlyServer::freeClientAsync(std::shared_ptr<AbstractFlyClient> flyClient) {
    if (flyClient->getFlags() & CLIENT_CLOSE_ASAP) {
        return;
    }

    flyClient->setFlags(CLIENT_CLOSE_ASAP);
    this->clientsToClose.push_back(flyClient);
}

void FlyServer::deleteFromPending(int fd) {
    std::list<std::shared_ptr<AbstractFlyClient>>::iterator iter =
            this->clientsPendingWrite.begin();
    for (; iter != this->clientsPendingWrite.end(); iter++) {
        if ((*iter)->getFd() == fd) {
            this->clientsPendingWrite.erase(iter);
            return;
        }
    }
}

void FlyServer::deleteFromAsyncClose(int fd) {
    std::list<std::shared_ptr<AbstractFlyClient>>::iterator iter =
            this->clientsToClose.begin();
    for (iter; iter != this->clientsToClose.end(); iter++) {
        if ((*iter)->getFd() == fd) {
            this->clientsToClose.erase(iter);
            return;
        }
    }
}

int FlyServer::getMaxClients() const {
    return this->maxClients;
}

/**
 * 将client从一切global list中删除掉
 * (除async delete列表之外, 否则可能导致无法删除)
 **/
void FlyServer::unlinkClient(std::shared_ptr<AbstractFlyClient> flyClient) {
    /** 在clients列表中删除，并删除该client对应的文件事件 */
    if (-1 != flyClient->getFd()) {
        std::list<std::shared_ptr<AbstractFlyClient>>::iterator iter =
                this->clients.begin();
        for (iter; iter != this->clients.end(); iter++) {
            if ((*iter)->getFd() != flyClient->getFd()) {
                this->clients.erase(iter);
                coordinator->getEventLoop()->deleteFileEvent(
                        flyClient->getFd(), ES_WRITABLE | ES_READABLE);
                coordinator->getEventLoop()->deleteFileEvent(
                        flyClient->getFd(), ES_READABLE);
                close(flyClient->getFd());
                flyClient->setFd(-1);
                break;
            }
        }
    }

    if (flyClient->IsPendingWrite()) {
        std::list<std::shared_ptr<AbstractFlyClient>>::iterator iter =
                this->clientsPendingWrite.begin();
        for (iter; iter != this->clientsPendingWrite.end(); iter++) {
            this->clientsPendingWrite.erase(iter);
            break;
        }
    }
}

void FlyServer::setupSignalHandlers() {
    signal(SIGTERM, sigShutDownHandlers);
    signal(SIGINT, sigShutDownHandlers);
}

void FlyServer::createSharedObjects() {
    for (int i = 0; i < OBJ_SHARED_BULKHDR_LEN; i++) {
        char buf[10];
        snprintf(buf, sizeof(buf), "*%d\r\n", i);
        mbulkHeader[i] = coordinator->getFlyObjStringFactory()
                ->getObject(new std::string(buf));
    }
}

bool FlyServer::isShutdownASAP() const {
    return this->shutdownASAP;
}

void FlyServer::setShutdownASAP(bool shutdownASAP) {
    this->shutdownASAP = shutdownASAP;
}

int FlyServer::prepareForShutdown(int flags) {
    AbstractFDBHandler *fdbHandler = coordinator->getFdbHandler();
    AbstractAOFHandler *aofHandler = coordinator->getAofHandler();

    /** 如果有fdb子进程存在，kill并且删掉fdb的临时文件 */
    if (fdbHandler->haveChildPid()) {
        pid_t fdbPid = fdbHandler->getChildPid();
        /** 向子进程发送SIGUSR1，子进程退出，但不标记异常 */
        kill(fdbPid, SIGUSR1);
        fdbHandler->deleteTempFile(fdbPid);
    }

    /** aof关闭前处理 */
    if (!aofHandler->IsStateOff()) {
        if (aofHandler->haveChildPid()) {
            /** 如果还没执行完aof步骤，不允许shutdown, 否则db数据会丢 */
            if (aofHandler->IsStateWaitRewrite()) {
                logHandler->logWarning("Writing initial AOF, can't exit.");
                return -1;
            }

            logHandler->logWarning(
                    "There is a child rewriting the AOF. Killing it!");
            /** 像子进程发送SIGUSR1，子进程退出，但不标记异常 */
            kill(aofHandler->getChildPid(), SIGUSR1);
            aofHandler->removeTempFile();
        }

        /**
         * 多加了个flush操作，防止有没有写入磁盘的aof缓存
         * */
        aofHandler->flush(true);
        aof_fsync(aofHandler->getFd());
    }

    /** 如果flag设置了save或者saveParamsCount>0, 则执行fdb持久化 */
    if ((fdbHandler->getSaveParamsCount() > 0 && !(flags & SHUTDOWN_NOSAVE))
        || (flags & SHUTDOWN_SAVE)) {
        if (fdbHandler->save() < 0) {
            logHandler->logWarning("Error trying to save the DB, can't exit.");
            return -1;
        }
    }

    /** 关闭所有listen socket，这样重启快一些 */
    closeListeningSockets(true);

    this->setShutdownASAP(false);
    this->logHandler->logWarning("now ready to exit, bye bye...");
    return 1;
}

void FlyServer::addCronLoops() {
    this->cronloops++;
}

uint64_t FlyServer::getCronLoops() const {
    return this->cronloops;
}

void FlyServer::sigShutDownHandlers(int sig) {
    extern AbstractCoordinator *coordinator;

    /**
     * 如果是连续收到两次(isShutdownASAP表明上次已经收到过信号了)SIGINT信号，
     * 表明用户想退出并且不持久化
     **/
    if (coordinator->getFlyServer()->isShutdownASAP()
        && SIGINT == sig) {
        coordinator->getFdbHandler()->deleteTempFile(getpid());
        exit(1);
    }

    /**
     * 如果正在进行持久化文件load，直接退出。
     *  持久化文件load只有在系统启动init server的时候，
     *  此时说明系统处于启动过程中，可以直接退出了
     **/
    if (coordinator->getFlyServer()->isLoading()) {
        exit(0);
    }

    coordinator->getFlyServer()->setShutdownASAP(true);
}

uint64_t FlyServer::getDirty() const {
    return this->dirty;
}

void FlyServer::databaseCron() {
    /** 删除flydb中的过期键 */
    this->activeExpireCycle(ACTIVE_EXPIRE_CYCLE_SLOW);

    /**
     * 如果不存在持久化后台子进程，则执行判断各db是否需要缩容（缩小db占用空间）:
     *     1.虽然deleteEntry时会做判断是否需要缩容（被动式缩容），
     *       但是假如一直没有删除操作，db一直占用内存过高而得不到清除，
     *       所以还是需要主动缩容。
     *     2.缩容操作不和持久化进程一起执行的目的，是为了防止io压力过大
     **/
    if (!coordinator->getFdbHandler()->haveChildPid()
        && !coordinator->getAofHandler()->haveChildPid()) {
        this->tryResizeDB();
    }
}

int serverCron(const AbstractCoordinator *coordinator,
               uint64_t id,
               void *clientData) {
    AbstractFlyServer *flyServer = coordinator->getFlyServer();

    /** 更新缓存时间 */
    flyServer->setNowt(time(NULL));

    /** 数据库循环操作 */
    flyServer->databaseCron();

    /** 释放所有异步删除的clients */
    flyServer->freeClientsInAsyncFreeList();

    /** 如果收到kill命令(SIGTERM信号)，执行fdb save操作 */
    if (coordinator->getFlyServer()->isShutdownASAP()) {
        if (flyServer->prepareForShutdown(SHUTDOWN_NOFLAGS)) {
            exit(0);
        }

        /** shutdown失败 */
        coordinator->getLogHandler()->logWarning(
                "SIGTERM received but errors trying to shut down the server, "
                "check the logs for more information");
        coordinator->getFlyServer()->setShutdownASAP(false);
    }

    // 如果有fdb或者aof子进程存在的话
    if (coordinator->getAofHandler()->haveChildPid()
        || coordinator->getFdbHandler()->haveChildPid()) {
        int statloc;
        pid_t pid = -1;

        /**
         * 当wait3的options设置为WNOHANG时，
         * 如果没有发现已退出的子进程时、不用等待而直接返回，返回值为0
         **/
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
                coordinator->getAofHandler()->backgroundSaveDone(
                        exitCode, bySignal);
            } else {
                coordinator->getLogHandler()->logWarning(
                        "Warning, detected child with unmatched pid: %ld", pid);
            }

            flyServer->updateDictResizePolicy();
            coordinator->getChildInfoPipe()->closeAll();
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
            if (coordinator->getFlyServer()->getDirty() > saveParam->changes
                && coordinator->getFdbHandler()->
                    lastSaveTimeGapGreaterThan(saveParam->seconds)
                && coordinator->getFdbHandler()->canBgsaveNow()
                && 1 == coordinator->getFdbHandler()->getLastBgsaveStatus()) {
                coordinator->getFdbHandler()->backgroundSave();
            }
        }

        /**
         * 不能因为haveChildPid这个条件判断与上面if判断重复，就不再做haveChildPid判断
         * 因为saveParam那里有可能会导致进行backgroundSave操作，导致haveChildPid条件发生改变
         **/
        /**
         * 处理被延迟了(schedule)的AOF操作:
         *    1.当前没有fdb child thread正在运行
         *    2.当前没有aof child thread挣在运行
         *    3.schedule被标记
         */
        if (!coordinator->getAofHandler()->haveChildPid()
            && !coordinator->getFdbHandler()->haveChildPid()
            && coordinator->getAofHandler()->isScheduled()) {
            if (1 == coordinator->getAofHandler()->rewriteBackground()) {
                coordinator->getAofHandler()->setScheduled(false);
            }
        }

        /**
         * 处理被延迟了(schedule)的FDB操作：
         *    1.没有fdb child thread正在运行
         *    2.没有aof child thread正在运行
         *    3.schedule被标记（即执行fdb时由于某些原因被延迟了）
         *    4.上次执行bgsave成功了, 或者失败了但是nowt-上次尝试执行fdb的时间>指定时间（CONFIG_BGSAVE_RETRY_DELAY）
         */
        if (!coordinator->getFdbHandler()->haveChildPid()
            && !coordinator->getAofHandler()->haveChildPid()
            && coordinator->getFdbHandler()->isBGSaveScheduled()
            && (coordinator->getFdbHandler()->canBgsaveNow()
                || 1 == coordinator->getFdbHandler()->getLastBgsaveStatus())) {
            // fdb被成功执行了，则下次不再schedule
            if (1 == coordinator->getFdbHandler()->backgroundSave()) {
                coordinator->getFdbHandler()->setBGSaveScheduled(false);
            }
        }

        /** 当前aof文件的大小达到rewrite的要求(minsize and growth percentage) */
        if (!coordinator->getAofHandler()->haveChildPid()
            && !coordinator->getFdbHandler()->haveChildPid()
            && coordinator->getAofHandler()->sizeMeetRewriteCondition()) {
            coordinator->getAofHandler()->rewriteBackground();
        }
    }

    /** 如果有被推迟的flush操作，则在每次循环的时候都试图执行一遍 */
    if (coordinator->getAofHandler()->flushPostponed()) {
        coordinator->getAofHandler()->flush(false);
    }

    /**
     * 如果上次flush产生失败（执行了部分写入），则重新执行flush，
     * 不需要每次cron loop都执行，每秒（1000ms）执行一次就可以了
     **/
    runWithPeriod(flyServer, 3000) {
        if (coordinator->getAofHandler()->lastWriteHasError()) {
            coordinator->getAofHandler()->flush(false);
        }
    }

    runWithPeriod(flyServer, 1000) {
        coordinator->getReplicationHandler()->cron();
    }

    flyServer->addCronLoops();
    std::cout << "serverCron is running "
              << flyServer->getCronLoops()
              << " times!" << std::endl;

    return 1000 / flyServer->getHz();
}

/** 用于判断当前cron loop下是否满足了周期要求：
 *  periodGap单位是毫秒
 **/
bool meetPeriod(AbstractFlyServer *flyServer, uint32_t periodGap) {
    int cronGap = 1000 / flyServer->getHz();

    /** 周期小于每次循环间隔时间，则当前循环肯定满足周期 */
    if (periodGap <= cronGap) {
        return true;
    }

    /** 达到period / cronGap表示循环几次时满足周期要求 */
    if (0 == flyServer->getCronLoops() % (periodGap / cronGap)) {
        return true;
    }

    return false;
}
