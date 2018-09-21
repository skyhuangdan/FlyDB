// Created by 赵立伟 on 2018/9/20.
//

#ifndef FLYDB_HASHTABLE_H
#define FLYDB_HASHTABLE_H

#include "DictEntry.h"
#include "DictType.h"


class HashTable {
 public:
    HashTable(DictType* const type, int size);
    // < 0加入失败，> 0加入成功
    int addEntry(void* key, void* val);
    DictEntry* findEntry(void* key);
    int deleteEntry(void* key);
    bool needExpand();
    bool hasKey(void* key);
    int getIndex(void* key);

private:
    DictEntry** table;
    unsigned long size;
    unsigned long used;
    unsigned long mask;
    DictType* const type;
};


#endif //FLYDB_HASHTABLE_H
