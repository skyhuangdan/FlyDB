//
// Created by 赵立伟 on 2018/9/18.
//

#ifndef FLYDB_FLYSERVER_H
#define FLYDB_FLYSERVER_H

#include <array>
#include <map>
#include <string>
#include <list>
#include "db/FlyDB.h"
#include "commandTable/CommandTable.h"
#include "flyClient/FlyClient.h"
#include "event/EventLoop.h"

const int DB_NUM = 4;
const std::string VERSION = "0.0.1";

class FlyServer {
public:
    FlyServer();

    // 初始化函数
    void init();
    // 获取server id
    int getPID();
    // 根据db id获取具体的db
    FlyDB* getDB(int dbID);
    // 获取版本号
    std::string getVersion();
    // 处理命令
    int dealWithCommand(std::string* command);
    // 事件循环处理
    void eventMain();

private:
    // 调整客户端描述符文件最大数量（即最大允许同时连接的client数量）
    void setMaxClientLimit();
    void clientCron(void);
    // 运行server的线程标识
    int pid;
    // db列表
    std::array<FlyDB*, DB_NUM> dbArray;
    // 版本号
    std::string version = VERSION;
    // 命令表
    CommandTable* commandTable;
    // client列表
    std::list<FlyClient *> clients;
    // tcp listening port
    int port;
    // 最大可同时连接的client数量
    int maxClients;
    // 事件循环处理器
    EventLoop *eventLoop;
};

#endif //FLYDB_FLYSERVER_H
