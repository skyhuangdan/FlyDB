//
// Created by 赵立伟 on 2018/11/29.
//

#ifndef FLYDB_FDBDEF_H
#define FLYDB_FDBDEF_H

#include <string>
#include "../../def.h"

struct FDBSaveInfo {
    // load和save时都有使用
    int replStreamDB;

    // 仅用于load
    int replIDIsSet = 0;                    // 标记replication id是否被设置了
    char replID[CONFIG_RUN_ID_SIZE + 1];    // Replication ID
    long long replOffset;                   // replication offset
};

enum FDBOpration {
    FDB_OPCODE_AUX = 250,
    FDB_OPCODE_RESIZEDB,
    FDB_OPCODE_EXPIRETIME_MS,
    FDB_OPCODE_EXPIRETIME,
    FDB_OPCODE_SELECTDB,
    FDB_OPCODE_EOF
};

/**
 *
 * 00|XXXXXX => 如果最高两位是00，则表示是六位数据，在XXXXXX中保存
 * 01|XXXXXX XXXXXXXX =>  如果是01, 则表示是14位数据, 6 bits + 8 bits of next byte
 * 10|000000 [32 bit integer] => 如果是10，则是32位数据，保存在了接下来的32bit中
 * 10|000001 [64 bit integer] => 64位数据，保存在了接下来的64bit中
 * 11|OBKIND this means: 剩余六位表示了自定义类型.
 *
 * */
enum FDBLenType {
    FDB_6BITLEN = 0,
    FDB_14BITLEN = 1,
    FDB_32BITLEN = 0x80,
    FDB_64BITLEN = 0x81,
    FDB_ENCVAL = 3
};

enum FDBEncoding {
    FDB_ENC_INT8 = 0,        // 8-bit
    FDB_ENC_INT16 = 1,       // 16-bit
    FDB_ENC_INT32 = 2,       // 32-bit
    FDB_ENC_LZF = 3          // string compressed with FASTLZ
};

enum FDBLoadType {
    FDB_LOAD_STRING = 0,
    FDB_LOAD_OBJECT
};

enum FDBChildType {
    RDB_CHILD_TYPE_NONE = 0,
    RDB_CHILD_TYPE_DISK = 1,
    RDB_CHILD_TYPE_SOCKET = 2
};

const int RDB_SAVE_NONE = 0;
const int RDB_SAVE_AOF_PREAMBLE = 1 << 0;

const int FDB_VERSION = 1;
const int FDB_STRINT_MAX_LEN = 30;

#endif //FLYDB_FDBDEF_H
