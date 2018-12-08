// Created by 赵立伟 on 2018/9/20.
//

#ifndef FLYDB_HASHTABLE_H
#define FLYDB_HASHTABLE_H

#include "DictEntry.h"
#include "Dict.h"
#include "DictDef.h"
#include "../../log/interface/AbstractLogHandler.h"

class HashTable {
 public:
    HashTable(const DictType* type, uint32_t size);
    virtual ~HashTable();

    int addEntry(void* key, void* val);
    DictEntry* findEntry(void* key);
    int deleteEntry(void* key);
    bool needExpand() const;
    bool needShrink() const;
    bool hasKey(void* key);
    uint32_t getIndex(void* key) const;
    uint32_t getIndex(uint32_t cursor) const;
    uint32_t getSize() const;
    uint32_t getUsed() const;
    bool isEmpty() const;
    DictEntry* getEntryBy(uint32_t index) const;
    void scanEntries(uint32_t index, scanProc proc, void* priv);
    uint32_t getMask() const;

private:
    DictEntry** table;
    uint32_t size;
    uint32_t used;
    uint32_t mask;
    const DictType* type;

    AbstractLogHandler *logHandler;
};

#endif //FLYDB_HASHTABLE_H
