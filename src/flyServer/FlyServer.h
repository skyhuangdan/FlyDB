//
// Created by 赵立伟 on 2018/9/18.
//

#ifndef FLYDB_FLYSERVER_H
#define FLYDB_FLYSERVER_H

#include <array>
#include <map>
#include <string>
#include <list>
#include <vector>
#include "../event/EventLoop.h"
#include "../def.h"
#include "../log/FileLogHandler.h"
#include "../aof/AOFHandler.h"
#include "../flyClient/interface/AbstractFlyClient.h"
#include "interface/AbstractFlyServer.h"
#include "../db/interface/AbstractFlyDB.h"
#include "../db/interface/AbstractFlyDBFactory.h"
#include "../utils/MiscTool.h"
#include "../pipe/Pipe.h"

class CommandTable;

/** 共享multi bulk len字段，其格式为: "*<value>\r\n" */
extern std::shared_ptr<FlyObj> mbulkHeader[OBJ_SHARED_BULKHDR_LEN];

int serverCron(const AbstractCoordinator *coordinator,
               uint64_t id,
               void *clientData);
bool meetPeriod(AbstractFlyServer *flyServer,
                uint32_t periodGap);

class FlyServer : public AbstractFlyServer {
public:
    FlyServer(const AbstractCoordinator *coordinator);
    ~FlyServer();
    void init(ConfigCache *configCache);                  // 初始化函数
    pid_t getPID();                                       // 获取server id
    std::string getVersion();                             // 获取版本号
    // 处理命令
    int dealWithCommand(std::shared_ptr<AbstractFlyClient> flyclient);
    int getHz() const;
    void setHz(int hz);
    time_t getNowt() const;
    void setNowt(time_t nowt);
    bool isShutdownASAP() const;
    void setShutdownASAP(bool shutdownASAP);
    int prepareForShutdown(int flags);
    void addCronLoops();
    uint64_t getCronLoops() const;

    /**
     * 网络相关
     **/
    char *getNeterr() const;
    size_t getClientMaxQuerybufLen() const;
    int64_t getStatNetInputBytes() const;
    void addToStatNetInputBytes(int64_t size);
    void closeListeningSockets(bool unlinkUnixSocket);

    /**
     * flydb
     */
    AbstractFlyDB* getFlyDB(int dbnum);
    uint8_t getFlyDBCount() const;
    void updateDictResizePolicy();
    void startToLoad();
    void stopLoad();
    bool isLoading() const;
    void activeExpireCycle(int type);
    void tryResizeDB();
    uint64_t addDirty(uint64_t count);
    uint64_t getDirty() const;
    void databaseCron();

    /**
     *  client相关
     */
    std::shared_ptr<AbstractFlyClient> createClient(int fd);
    int freeClient(std::shared_ptr<AbstractFlyClient> flyClient);
    void addToClientsPendingToWrite(std::shared_ptr<AbstractFlyClient> flyClient);
    int handleClientsWithPendingWrites();
    void freeClientAsync(std::shared_ptr<AbstractFlyClient> flyClient);
    void freeClientsInAsyncFreeList();
    int getMaxClients() const;
    void unlinkClient(std::shared_ptr<AbstractFlyClient> flyClient);

private:
    /** 调整客户端描述符文件最大数量（即最大允许同时连接的client数量）*/
    void setMaxClientLimit();
    /** 打开监听socket，用于监听用户命令 */
    int listenToPort();
    void deleteFromPending(int fd);
    void deleteFromAsyncClose(int fd);
    void loadDataFromDisk();
    void loadFromConfig(ConfigCache *configCache);
    void setupSignalHandlers();
    void createSharedObjects();

    static void sigShutDownHandlers(int sig);

    /** General */
    pid_t pid;                                        // 运行server的进程标识
    AbstractFlyDBFactory *flyDBFactory;
    std::array<AbstractFlyDB*, DB_NUM> dbArray;       // db列表
    std::string version = VERSION;                    // 版本号
    CommandTable* commandTable;                       // 命令表
    int hz;                                           // serverCron运行频率
    time_t nowt;                                      // 系统当前时间
    uint32_t lruclock;                                // LRU
    bool shutdownASAP = false;

    /** flydb相关 */
    bool loading = false;
    time_t loadingStartTime = 0;
    uint64_t loadBytes = 0;
    uint64_t loadTotalBytes = 0;
    uint8_t currentExpireDB = 0;
    uint8_t currentShrinkDB = 0;
    bool timelimitExit = false;
    uint64_t lastFastCycle = 0;
    uint64_t dirty = 0;

    /**
     *  网络相关
     */
    int port;                                 // tcp listening port
    std::vector<int> ipfd;                    // TCP socket fd
    std::vector<std::string> bindAddr;        // 绑定地址
    int tcpBacklog;                           // TCP listen() backlog
    char *neterr;                             // 网络error buffer
    const char *unixsocket;                   // UNIX socket path
    mode_t unixsocketperm;                    // UNIX socket permission
    int usfd;                                 // Unix socket file descriptor
    int64_t statNetInputBytes;                // 该server从网络获取的byte数量
    int tcpKeepAlive;

    /**
     * client相关
     */
    /** 最大可同时连接的client数量 */
    int maxClients;
    uint64_t nextClientId;
    pthread_mutex_t nextClientIdMutex;
    /** client列表 */
    std::list<std::shared_ptr<AbstractFlyClient>> clients;
    /** 需要install write handler */
    std::list<std::shared_ptr<AbstractFlyClient>> clientsPendingWrite;
    /** 异步关闭的client链表 */
    std::list<std::shared_ptr<AbstractFlyClient>> clientsToClose;
    /** 由于超过了maxclients而拒绝连接的次数 */
    uint64_t statRejectedConn;
    /** client buff最大长度 */
    size_t clientMaxQuerybufLen;
    uint64_t cronloops = 0;

    AbstractLogHandler *logHandler;
    const AbstractCoordinator *coordinator;
};

#endif //FLYDB_FLYSERVER_H
