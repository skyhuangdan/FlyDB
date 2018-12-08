//
// Created by 赵立伟 on 2018/9/18.
//

#ifndef FLYDB_FLYSERVER_H
#define FLYDB_FLYSERVER_H

#include <array>
#include <map>
#include <string>
#include <list>
#include <pthread.h>
#include "../event/EventLoop.h"
#include "../../def.h"
#include "../log/FileLogHandler.h"
#include "../aof/AOFHandler.h"
#include "../aof/AOFDef.h"
#include "../flyClient/interface/AbstractFlyClient.h"
#include "interface/AbstractFlyServer.h"
#include "../db/interface/AbstractFlyDB.h"
#include "../db/interface/AbstractFlyDBFactory.h"
#include "../utils/MiscTool.h"

const int DB_NUM = 4;
const std::string VERSION = "0.0.1";

int serverCron(const AbstractCoordinator *coordinator, uint64_t id, void *clientData);

class CommandTable;

class FlyServer : public AbstractFlyServer {
public:
    FlyServer(const AbstractCoordinator *coordinator);
    ~FlyServer();
    void init(ConfigCache *configCache);                  // 初始化函数
    int getPID();                                         // 获取server id
    std::string getVersion();                             // 获取版本号
    int dealWithCommand(AbstractFlyClient *flyclient);    // 处理命令
    int getHz() const;
    void setHz(int hz);
    time_t getNowt() const;
    void setNowt(time_t nowt);

    /**
     * 网络相关
     **/
    char *getNeterr() const;
    size_t getClientMaxQuerybufLen() const;
    int64_t getStatNetInputBytes() const;
    void addToStatNetInputBytes(int64_t size);
    /**
     * flydb
     */
    AbstractFlyDB* getFlyDB(int dbnum);
    uint8_t getFlyDBCount() const;

    /**
     *  client相关
     */
    AbstractFlyClient* createClient(int fd);
    int deleteClient(int fd);
    void addToClientsPendingToWrite(AbstractFlyClient *flyClient);
    int handleClientsWithPendingWrites();
    void freeClientAsync(AbstractFlyClient *flyClient);
    void freeClientsInAsyncFreeList();
    int getMaxClients() const;

private:
    /** 调整客户端描述符文件最大数量（即最大允许同时连接的client数量）*/
    void setMaxClientLimit();
    /** 打开监听socket，用于监听用户命令 */
    int listenToPort();
    void deleteFromPending(int fd);
    void deleteFromAsyncClose(int fd);
    void loadDataFromDisk();
    void loadFromConfig(ConfigCache *configCache);

    int pid;                                          // 运行server的线程标识
    AbstractFlyDBFactory *flyDBFactory;
    std::array<AbstractFlyDB*, DB_NUM> dbArray;       // db列表
    std::string version = VERSION;                    // 版本号
    CommandTable* commandTable;                       // 命令表
    int hz;                                           // serverCron运行频率
    time_t nowt;                                      // 系统当前时间
    uint32_t lruclock;                                // LRU

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
    std::list<AbstractFlyClient *> clients;
    /** 需要install write handler */
    std::list<AbstractFlyClient*> clientsPendingWrite;
    /** 异步关闭的client链表 */
    std::list<AbstractFlyClient*> clientsToClose;
    /** 由于超过了maxclients而拒绝连接的次数 */
    uint64_t statRejectedConn;
    /** client buff最大长度 */
    size_t clientMaxQuerybufLen;

    /**
     * AOF持久化
     * */
    AOFState aofState;

    MiscTool *miscTool;
    AbstractLogHandler *logHandler;
    const AbstractCoordinator *coordinator;
};

#endif //FLYDB_FLYSERVER_H
