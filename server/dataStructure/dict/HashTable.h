// Created by 赵立伟 on 2018/9/20.
//

#ifndef FLYDB_HASHTABLE_H
#define FLYDB_HASHTABLE_H

#include "DictEntry.h"
#include "Dict.h"
#include "DictDef.h"

class HashTable {
 public:
    HashTable(const DictType& type, unsigned long size);
    virtual ~HashTable();

    int addEntry(void* key, void* val);
    DictEntry* findEntry(void* key);
    int deleteEntry(void* key);
    bool needExpand() const;
    bool needShrink() const;
    bool hasKey(void* key);
    unsigned long getIndex(void* key) const;
    unsigned long getIndex(unsigned long cursor) const;
    unsigned long getSize() const;
    bool isEmpty() const;
    DictEntry* getEntryBy(unsigned long index) const;
    void scanEntries(unsigned long index, scanProc proc, void* priv);
    unsigned long getMask() const;

private:
    DictEntry** table;
    unsigned long size;
    unsigned long used;
    unsigned long mask;
    const DictType& type;
};


#endif //FLYDB_HASHTABLE_H
