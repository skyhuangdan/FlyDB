//
// Created by 赵立伟 on 2018/11/29.
//

#ifndef FLYDB_FDBDEF_H
#define FLYDB_FDBDEF_H

#include <string>

struct FDBSaveInfo {
    // load和save时都有使用
    int replStreamDB;

    // 仅用于load
    int replIDIsSet;                        // 标记replication id是否被设置了
    std::string replID;                     // replication id
    //char replID[CONFIG_RUN_ID_SIZE+1];    // Replication ID
    long long replOffset;                   // replication offset
};

enum FDBOpration {
    RDB_OPCODE_AUX = 250,
    RDB_OPCODE_RESIZEDB,
    RDB_OPCODE_EXPIRETIME_MS,
    RDB_OPCODE_EXPIRETIME,
    RDB_OPCODE_SELECTDB,
    RDB_OPCODE_EOF
};

const int FDB_VERSION = 8;

#endif //FLYDB_FDBDEF_H
