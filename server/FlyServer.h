//
// Created by 赵立伟 on 2018/9/18.
//

#ifndef FLYDB_FLYSERVER_H
#define FLYDB_FLYSERVER_H

#include <sys/_types/_pid_t.h>
#include <array>

#include "FlyDB.h"

const int DB_NUM = 4;

class FlyServer {
public:
    FlyServer();

    // 初始化函数
    void init();

    // 获取server id
    int getPID();
    FlyDB* getDB(int dbID);


private:
    // server的唯一标识
    pid_t pid;

    // db列表
    std::array<FlyDB*, DB_NUM> dbArray;
};


#endif //FLYDB_FLYSERVER_H
