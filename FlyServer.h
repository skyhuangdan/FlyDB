//
// Created by 赵立伟 on 2018/9/18.
//

#ifndef FLYDB_FLYSERVER_H
#define FLYDB_FLYSERVER_H

#include <array>
#include <map>
#include <string>
#include "FlyDB.h"
#include "CommandEntry.h"

const int DB_NUM = 4;

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
    // 处理command
    int dealWithCommand(std::string command);

private:
    void initCommandTable();

private:
    // 运行server的线程标识
    int pid;
    // db列表
    std::array<FlyDB*, DB_NUM> dbArray;
    // 命令表
    std::map<std::string, CommandEntry*> commandTable;
    // 版本号
    std::string version = "0.0.1";
};

#endif //FLYDB_FLYSERVER_H
