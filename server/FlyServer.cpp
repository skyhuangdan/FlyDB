//
// Created by 赵立伟 on 2018/9/18.
//

#include <iostream>
#include <syslog.h>
#include "FlyServer.h"
#include "commandTable/commandTable.h"
#include "config/config.h"
#include "atomic/AtomicHandler.h"
#include "net/NetDef.h"
#include "utils/MiscTool.h"
#include "net/NetHandler.h"
#include "log/LogHandler.h"
#include "flyClient/ClientDef.h"
#include "fdb/FDBHandler.h"
#include "aof/AOFDef.h"

configMap loglevelMap[] = {
        {"debug",   LL_DEBUG},
        {"verbose", LL_VERBOSE},
        {"notice",  LL_NOTICE},
        {"warning", LL_WARNING},
        {NULL, 0}
};

configMap syslogFacilityMap[] = {
        {"user",    LOG_USER},
        {"local0",  LOG_LOCAL0},
        {"local1",  LOG_LOCAL1},
        {"local2",  LOG_LOCAL2},
        {"local3",  LOG_LOCAL3},
        {"local4",  LOG_LOCAL4},
        {"local5",  LOG_LOCAL5},
        {"local6",  LOG_LOCAL6},
        {"local7",  LOG_LOCAL7},
        {NULL, 0}
};

FlyServer::FlyServer() {
    // init db array
    for (int i = 0; i < DB_NUM; i++) {
        this->dbArray[i] = new FlyDB();
        if (NULL == this->dbArray.at(i)) {
            std::cout << "error to create FlyDB[" << i << "]" << std::endl;
            exit(1);
        }
    }

    // init command table
    this->commandTable = new CommandTable(this);
    // server端口
    this->port = CONFIG_DEFAULT_SERVER_PORT;
    // unix domain socket
    this->unixsocket = NULL;
    this->unixsocketperm = CONFIG_DEFAULT_UNIX_SOCKET_PERM;
    // 设置最大客户端数量
    setMaxClientLimit();
    // 时间循环处理器
    this->eventLoop = new EventLoop(this, this->maxClients + CONFIG_FDSET_INCR);
    this->eventLoop->createTimeEvent(1, serverCron, NULL, NULL);
    // serverCron运行频率
    this->hz = CONFIG_CRON_HZ;
    this->neterr = new char[NET_ERR_LEN];
    // keep alive
    this->tcpKeepAlive = CONFIG_DEFAULT_TCP_KEEPALIVE;
    // 拒绝连接次数设置为0
    this->statRejectedConn = 0;
    // next client id
    this->nextClientId = 1;
    pthread_mutex_init(&this->nextClientIdMutex, NULL);
    // 当前时间
    this->nowt = time(NULL);
    this->clientMaxQuerybufLen = PROTO_MAX_QUERYBUF_LEN;
    // log相关
    this->verbosity = CONFIG_DEFAULT_VERBOSITY;
    this->syslogEnabled = CONFIG_DEFAULT_SYSLOG_ENABLED;
    this->logfile = strdup(CONFIG_DEFAULT_LOGFILE.c_str());
    this->syslogIdent = strdup(CONFIG_DEFAULT_SYSLOG_IDENT.c_str());
    // fdb相关
    this->fdbFile = strdup(CONFIG_DEFAULT_FDB_FILENAME.c_str());
    this->fdbHandler = FDBHandler::getInstance();

    this->miscTool = MiscTool::getInstance();
    this->netHandler = NetHandler::getInstance();
}

FlyServer::~FlyServer() {
    for (int i = 0; i < DB_NUM; i++) {
        delete this->dbArray.at(i);
    }
    delete this->commandTable;
    delete this->eventLoop;
    delete[] this->neterr;
    closelog();
}

void FlyServer::init(int argc, char **argv) {
    // 加载配置文件中配置
    std::string fileName;
    if (1 == this->miscTool->getAbsolutePath(this->configfile, fileName)) {
        loadConfig(fileName);
    }

    // 打开监听socket，用于监听用户命令
    this->listenToPort();

    // 打开Unix domain socket
    if (NULL != this->unixsocket) {
        unlink(this->unixsocket);       // 如果存在，则删除unixsocket文件
        this->usfd = this->netHandler->unixServer(this->neterr,
                this->unixsocket, this->unixsocketperm, this->tcpBacklog);
        if (-1 == this->usfd) {
            std::cout << "Opening Unix Domain Socket: "
                         << this->neterr << std::endl;
            exit(1);
        }
        this->netHandler->setBlock(NULL, this->usfd, 0);
    }

    // 创建定时任务，用于创建客户端连接
    for (auto fd : this->ipfd) {
        if (-1 == this->eventLoop->createFileEvent(
                fd, ES_READABLE, acceptTcpHandler, NULL)) {
            exit(1);
        }
    }

    // syslog
    if (this->syslogEnabled) {
        openlog(this->syslogIdent, LOG_PID | LOG_NDELAY | LOG_NOWAIT,
                this->syslogFacility);
    }

    // LogHandler放在最后，因为初始化需要flyServer的配置, 最好等其初始化完毕
    LogHandler::init(this->logfile, this->syslogEnabled, this->verbosity);
    this->logHandler = LogHandler::getInstance();

    // 从fdb或者aof中加载数据
    loadDataFromDisk();

    return;
}

int FlyServer::getPID() {
    return this->pid;
}

FlyDB* FlyServer::getDB(int dbID) {
    return this->dbArray.at(dbID);
}

std::string FlyServer::getVersion() {
    return this->version;
}

int FlyServer::dealWithCommand(FlyClient *flyclient) {
    return this->commandTable->dealWithCommand(flyclient);
}

void FlyServer::eventMain() {
    this->eventLoop->eventMain();
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

FlyClient* FlyServer::createClient(int fd) {
    if (fd <= 0) {
        return NULL;
    }

    // 超过了客户端最大数量
    if (this->clients.size() >= this->maxClients) {
        this->statRejectedConn++;
        return NULL;
    }

    // create FlyClient
    FlyClient *flyClient = new FlyClient(fd, this);
    uint64_t clientId = 0;
    atomicGetIncr(this->nextClientId, clientId, 1);
    flyClient->setId(clientId);

    // 设置读socket，并为其创建相应的file event
    this->netHandler->setBlock(NULL, fd, 0);
    this->netHandler->setTcpNoDelay(NULL, fd, 1);
    if (this->tcpKeepAlive > 0) {
        this->netHandler->keepAlive(NULL, fd, this->tcpKeepAlive);
    }
    if (-1 == this->eventLoop->createFileEvent(
            fd, ES_READABLE, readQueryFromClient, flyClient)) {
        delete flyClient;
        return NULL;
    }

    // 加入到clients队列中
    this->clients.push_back(flyClient);

    return flyClient;
}

int FlyServer::deleteClient(int fd) {
    std::list<FlyClient *>::iterator iter = this->clients.begin();
    for (iter; iter != this->clients.end(); iter++) {
        if (fd == (*iter)->getFd()) {
            // 删除file event
            this->eventLoop->deleteFileEvent(fd, ES_READABLE | ES_WRITABLE);

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

int FlyServer::getVerbosity() const {
    return this->verbosity;
}

char *FlyServer::getLogfile() const {
    return this->logfile;
}

int FlyServer::getSyslogEnabled() const {
    return this->syslogEnabled;
}

char *FlyServer::getSyslogIdent() const {
    return this->syslogIdent;
}

int FlyServer::getSyslogFacility() const {
    return this->syslogFacility;
}

NetHandler *FlyServer::getNetHandler() const {
    return netHandler;
}

void FlyServer::addToClientsPendingToWrite(FlyClient *flyClient) {
    this->clientsPendingWrite.push_back(flyClient);
}

int FlyServer::handleClientsWithPendingWrites() {
    if (0 == this->clientsPendingWrite.size()) {
        return 0;
    }

    std::list<FlyClient*>::iterator iter = this->clientsPendingWrite.begin();
    for (iter; iter != this->clientsPendingWrite.end(); iter++) {
        // 先清除标记，清空了该标记才回保证该客户端再次加入到clientsPendingWrite里；
        // 否则无法加入。也就无法处理其输出
        (*iter)->delFlag(~CLIENT_PENDING_WRITE);

        // 先直接发送，如果发送不完，再创建文件事件异步发送
        if (-1 == this->netHandler->writeToClient(
                this->eventLoop, this, *iter, 0)) {
            continue;
        }

        // 异步发送
        if (!(*iter)->hasNoPending()
            && -1 == eventLoop->createFileEvent((*iter)->getFd(), ES_WRITABLE,
                                                sendReplyToClient, *iter)) {
            freeClientAsync(*iter);
        }
    }

    int pendingCount = this->clientsPendingWrite.size();
    this->clientsPendingWrite.clear();
    return pendingCount;
}

void FlyServer::freeClientAsync(FlyClient *flyClient) {
    if (flyClient->getFlags() & CLIENT_CLOSE_ASAP) {
        return;
    }

    flyClient->setFlags(CLIENT_CLOSE_ASAP);
    this->clientsToClose.push_back(flyClient);
}

void FlyServer::setMaxClientLimit() {
    this->maxClients = CONFIG_DEFAULT_MAX_CLIENTS;
    int maxFiles = this->maxClients + CONFIG_MIN_RESERVED_FDS;
    rlimit limit;

    // 获取当前进程可打开的最大文件描述符
    if (getrlimit(RLIMIT_NOFILE, &limit) == -1) {
        // 如果获取失败, 按照进程中最大文件数量为1024计算(内核默认1024), 重置maxClients
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

void FlyServer::loadConfig(const std::string& fileName) {
    char buf[CONFIG_MAX_LINE + 1];
    std::string config;

    if (fileName.length() != 0) {
        FILE *fp = NULL;
        if ('-' == fileName[0] && '\0' == fileName[1]) {
            fp = stdin;
        } else {
            if (NULL == (fp = fopen(fileName.c_str(), "r"))) {
                exit(1);
            }
        }

        // 从配置文件中依次读取配置 --> config
        while (fgets(buf, CONFIG_MAX_LINE + 1, fp) != NULL) {
            config += buf;
        }

        // 读取完毕，如果不是stdin，则关闭文件
        if (fp != stdin) {
            fclose(fp);
        }
    }

    loadConfigFromString(config);
}

void FlyServer::loadConfigFromString(const std::string& config) {
    // 将文件分隔成行
    std::string delim = "\n";
    std::vector<std::string> lines;
    this->miscTool->spiltString(config, delim, lines);

    // 依次处理每行
    for (auto line : lines) {
        if (0 == line.size() || '#' == line[0]) {
            continue;
        }
        loadConfigFromLineString(line);
    }
}

void FlyServer::loadConfigFromLineString(const std::string &line) {
    // 截取words
    std::vector<std::string> words;
    this->miscTool->spiltString(line, " ", words);
    if (0 == words.size()) {
        return;
    }

    if (0 == words[0].compare("port") && 2 == words.size()) {
        int port = atoi(words[1].c_str());
        if (0 <= port && port <= 65535) {
            this->port = port;
        }
    } else if (0 == words[0].compare("bind")) {
        int addressCount = words.size() - 1;
        if (addressCount > CONFIG_BINDADDR_MAX) {
            std::cout << "Too many bind addresses specified!" << std::endl;
            exit(1);
        }
        for (int j = 0; j < addressCount; j++) {
            this->bindAddr.push_back(words[j + 1]);
        }
    } else if (0 == words[0].compare("unixsocket") && 2 == words.size()) {
        this->unixsocket = strdup(words[1].c_str());
    } else if (0 == words[0].compare("unixsocketperm") && 2 == words.size()) {
        this->unixsocketperm = (mode_t) strtol(words[1].c_str(), NULL, 0);
        if (this->unixsocketperm > 0777) {
            std::cout << "Invalid socket file permissions" << std::endl;
            exit(1);
        }
    } else if (0 == words[0].compare("tcp-keepalive") && 2 == words.size()) {
        this->tcpKeepAlive = atoi(words[1].c_str());
        if (this->tcpKeepAlive < 0) {
            std::cout << "Invalid tcp-keepalive value" << std::endl;
            exit(1);
        }
    } else if (0 == words[0].compare("logfile") && 2 == words.size()) {
        FILE *logfd;
        free(this->logfile);
        this->logfile = strdup(words[1].c_str());
        if ('\0' != this->logfile[0]) {
            // 尝试打开一次，查看是否可以正常打开
            logfd = fopen(this->logfile, "a");
            if (NULL == logfd) {
                std::cout << "Can not open log file: "
                             << this->logfile << std::endl;
                exit(1);
            }
            fclose(logfd);
        }
    } else if (0 == words[0].compare("syslog-enabled") && 2 == words.size()) {
        if (-1 == (this->syslogEnabled =
                this->miscTool->yesnotoi(words[1].c_str()))) {
            std::cout << "syslog-enabled must be 'yes(YES)' or 'no(NO)'"
                         << std::endl;
            exit(1);
        }
    } else if (0 == words[0].compare("syslog-ident") && 2 == words.size()) {
        if (this->syslogIdent) {
            free(this->syslogIdent);
        }
        this->syslogIdent = strdup(words[1].c_str());
    } else if (0 == words[0].compare("loglevel") && 2 == words.size()) {
        this->verbosity = configMapGetValue(loglevelMap, words[1].c_str());
        if (INT_MIN == this->verbosity) {
            std::cout << "Invalid log level. "
                         "Must be one of debug, verbose, notice, warning"
                         << std::endl;
            exit(1);
        }
    } else if (0 == words[0].compare("syslog-facility") && 2 == words.size()) {
        this->syslogFacility =
                configMapGetValue(syslogFacilityMap, words[1].c_str());
        if (INT_MIN == this->syslogFacility) {
            std::cout << "Invalid log facility. "
                         "Must be one of USER or between LOCAL0-LOCAL7"
                         << std::endl;
            exit(1);
        }
    } else if (0 == words[0].compare("dbfilename") && 2 == words.size()) {
        FILE *fdbfd;
        free(this->fdbFile);
        this->fdbFile = strdup(words[1].c_str());

        // 尝试打开一次，查看是否可以正常打开
        fdbfd = fopen(this->fdbFile, "a");
        if (NULL == fdbfd) {
            std::cout << "Can not open fdb file: " << this->fdbFile << std::endl;
            exit(1);
        }
        fclose(fdbfd);
    } else if (0 == words[0].compare("appendonly") && words.size() == 2) {
        int yes;
        if ((yes = this->miscTool->yesnotoi(words[1].c_str())) == -1) {
            std::cout <<  "argument must be 'yes' or 'no'";
            exit(1);
        }
        this->aofState = yes ? AOF_ON : AOF_OFF;
    }

}

int FlyServer::listenToPort() {
    int fd;
    // try to bind all to IPV4 and IPV6
    if (0 == this->bindAddr.size()) {
        int success = 0;
        // try to set *(any address) to ipv6
        fd = this->netHandler->tcp6Server(this->neterr,
                this->port, NULL, this->tcpBacklog);
        if (fd != -1) {
            // set nonblock
            this->netHandler->setBlock(NULL, fd, 0);
            this->ipfd.push_back(fd);
            success++;
        }

        // try to set *(any address) to ipv4
        fd = this->netHandler->tcpServer(this->neterr,
                this->port, NULL, this->tcpBacklog);
        if (fd != -1) {
            // set nonblock
            this->netHandler->setBlock(NULL, fd, 0);
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
                fd = this->netHandler->tcp6Server(this->neterr,
                        this->port, addr.c_str(), this->tcpBacklog);
            } else {
                fd = this->netHandler->tcpServer(this->neterr,
                        this->port, addr.c_str(), this->tcpBacklog);
            }
            if (-1 == fd) {
                return -1;
            }
            this->netHandler->setBlock(NULL, fd, 0);
            this->ipfd.push_back(fd);
        }
    }

    return 1;
}

int FlyServer::configMapGetValue(configMap *config, const char *name) {
    while (config->name != NULL) {
        if (!strcasecmp(config->name, name)) {
            return config->value;
        }
        config++;
    }
    return INT_MIN;
}

void FlyServer::deleteFromPending(int fd) {
    std::list<FlyClient*>::iterator iter = this->clientsPendingWrite.begin();
    for (iter; iter != this->clientsPendingWrite.end(); iter++) {
        if ((*iter)->getFd() == fd) {
            this->clientsPendingWrite.erase(iter);
            return;
        }
    }
}

void FlyServer::deleteFromAsyncClose(int fd) {
    std::list<FlyClient*>::iterator iter = this->clientsToClose.begin();
    for (iter; iter != this->clientsToClose.end(); iter++) {
        if ((*iter)->getFd() == fd) {
            this->clientsToClose.erase(iter);
            return;
        }
    }
}

void FlyServer::loadDataFromDisk() {
    // 如果开启了AOF，则优先从AOF中加载持久化数据，否则从FDB中加载
    if (AOF_ON == this->aofState) {
        // load from append only fiile
    } else {
        // load from fdb
    }

}

void FlyServer::freeClientsInAsyncFreeList() {
    for (auto client : this->clientsToClose) {
        deleteClient(client->getFd());
    }

    this->clientsToClose.clear();
}

int serverCron(EventLoop *eventLoop, uint64_t id, void *clientData) {
    FlyServer *flyServer = eventLoop->getFlyServer();

    // 设置当前时间
    flyServer->setNowt(time(NULL));

    // 释放所有异步删除的clients
    flyServer->freeClientsInAsyncFreeList();

    static int times = 0;
    std::cout << "serverCron is running " << times++ << " times!" << std::endl;

    return 1000 / flyServer->getHz();
}
