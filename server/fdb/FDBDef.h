//
// Created by 赵立伟 on 2018/11/29.
//

#ifndef FLYDB_FDBDEF_H
#define FLYDB_FDBDEF_H

#include <string>

struct fdbSaveInfo {
    // load和save时都有使用
    int replStreamDB;

    // 仅用于load
    int replIDIsSet;                        // 标记replication id是否被设置了
    std::string replID;                     // replication id
    //char replID[CONFIG_RUN_ID_SIZE+1];    // Replication ID
    long long replOffset;                   // replication offset
};

#endif //FLYDB_FDBDEF_H
