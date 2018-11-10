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
#include "db/FlyDB.h"
#include "commandTable/CommandTable.h"
#include "flyClient/FlyClient.h"
#include "event/EventLoop.h"
#include "config.h"

const int DB_NUM = 4;
const std::string VERSION = "0.0.1";

int serverCron(EventLoop *eventLoop, uint64_t id, void *clientData);

class FlyServer {
public:
    FlyServer();
    ~FlyServer();
    void init(int argc, char **argv);            // 初始化函数
    int getPID();                                // 获取server id
    FlyDB* getDB(int dbID);                      // 根据db id获取具体的db
    std::string getVersion();                    // 获取版本号
    int dealWithCommand(FlyClient *flyclient);          // 处理命令
    void eventMain();                            // 事件循环处理
    int getHz() const;
    void setHz(int hz);
    char *getNeterr() const;
    FlyClient* createClient(int fd);
    int deleteClient(int fd);

private:
    void setMaxClientLimit();                 // 调整客户端描述符文件最大数量（即最大允许同时连接的client数量）
    void loadConfig(const std::string &fileName);
    void loadConfigFromString(const std::string& config);
    void loadConfigFromLineString(const std::string &line);
    int listenToPort();                       // 打开监听socket，用于监听用户命令

    int pid;                                  // 运行server的线程标识
    std::array<FlyDB*, DB_NUM> dbArray;       // db列表
    std::string version = VERSION;            // 版本号
    CommandTable* commandTable;               // 命令表
    std::list<FlyClient *> clients;           // client列表
    int port;                                 // tcp listening port
    int maxClients;                           // 最大可同时连接的client数量
    EventLoop *eventLoop;                     // 事件循环处理器
    int hz;                                   // serverCron运行频率
    std::vector<int> ipfd;                    // TCP socket fd
    std::vector<std::string> bindAddr;        // 绑定地址
    int tcpBacklog;                           // TCP listen() backlog
    char *neterr;                             // 网络error buffer
    const char *unixsocket;                   // UNIX socket path
    mode_t unixsocketperm;                    // UNIX socket permission
    int usfd;                                 // Unix socket file descriptor
    int tcpKeepAlive;
    uint64_t statRejectedConn;               // 由于超过了maxclients而拒绝连接的次数
    uint64_t nextClientId;
    pthread_mutex_t nextClientIdMutex;
};

#endif //FLYDB_FLYSERVER_H
