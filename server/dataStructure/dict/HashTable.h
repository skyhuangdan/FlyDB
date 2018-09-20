// Created by 赵立伟 on 2018/9/20.
//

#ifndef FLYDB_HASHTABLE_H
#define FLYDB_HASHTABLE_H

#include "DictEntry.h"
#include "DictType.h"

const int HASH_TABLE_INITIAL_SIZE = 4;

class HashTable {
 public:
    HashTable(class DictType* type);
    // < 0加入失败，> 0加入成功
    int addEntry(void* key, void* val);
    DictEntry* findEntry(void* key);
    bool needExpand();
    bool hasKey(void* key, DictType* type);
    bool hasSameKey(void* key);

    class DictEntry** table;
    unsigned long size;
    unsigned long used;
    class DictType* type;
};


#endif //FLYDB_HASHTABLE_H
