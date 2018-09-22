// Created by 赵立伟 on 2018/9/20.
//

#ifndef FLYDB_HASHTABLE_H
#define FLYDB_HASHTABLE_H

#include "DictEntry.h"
#include "DictType.h"


class HashTable {
 public:
    HashTable(DictType* const type, unsigned long size);
    virtual ~HashTable();

    // < 0加入失败，> 0加入成功
    int addEntry(void* key, void* val);
    DictEntry* findEntry(void* key);
    int deleteEntry(void* key);
    bool needExpand() const;
    bool needShrink() const;
    bool hasKey(void* key);
    unsigned long getIndex(void* key) const;
    unsigned long getSize() const;
    unsigned long isEmpty() const;
    DictEntry* getEntryBy(int index) const;

private:
    DictEntry** table;
    unsigned long size;
    unsigned long used;
    unsigned long mask;
    DictType* const type;
};


#endif //FLYDB_HASHTABLE_H
