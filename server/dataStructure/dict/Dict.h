//
// Created by 赵立伟 on 2018/9/20.
//

#ifndef FLYDB_DICT_H
#define FLYDB_DICT_H

#include <map>
#include <array>
#include "HashTable.h"
#include "DictDef.h"

uint64_t dictStrHash(const void *key);
void dictStrDestructor(void *val);
int dictStrKeyCompare(const void *key1, const void *key2);

class MiscTool;
class LogHandler;

class Dict {
public:
    Dict(const DictType* type);
    virtual ~Dict();

    int addEntry(void* key, void* val);
    int replace(void* key, void* val);
    DictEntry* findEntry(void* key);
    void* fetchValue(void* key);
    int deleteEntry(void* key);
    bool isRehashing() const;
    void rehashSteps(uint32_t steps);
    uint32_t dictScan(uint32_t cursor, uint32_t steps, scanProc proc, void *priv);
    uint32_t dictScanOneStep(uint32_t cursor, scanProc proc, void *priv);
    int expand(uint32_t size);              // 扩容/缩容
    
private:
    uint32_t nextPower(uint32_t num);
    uint32_t revBits(uint32_t bits);
    std::array<class HashTable*, 2> ht;
    const DictType* type;
    int64_t rehashIndex = -1;

    LogHandler *logHandler;
};


#endif //FLYDB_DICT_H
