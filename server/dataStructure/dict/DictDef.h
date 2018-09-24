//
// Created by 赵立伟 on 2018/9/22.
//

#ifndef FLYDB_DICTDEF_H
#define FLYDB_DICTDEF_H

typedef void (*scanProc)(void* priv, void* key, void* val);

const uint32_t HASH_TABLE_INITIAL_SIZE = 4;      // hash table初始大小
const uint8_t NEED_REHASH_RATIO = 5;            // rehash的触发比例

#endif //FLYDB_DICTDEF_H
