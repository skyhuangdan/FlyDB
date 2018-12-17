//
// Created by 赵立伟 on 2018/9/20.
//

#ifndef FLYDB_DICT_H
#define FLYDB_DICT_H

#include <map>
#include <array>
#include "HashTable.cpp"
#include "DictDef.h"
#include "../../log/interface/AbstractLogFactory.h"

template<class KEY, class VAL>
class Dict {
public:
    Dict();
    virtual ~Dict();

    int addEntry(KEY* key, VAL* val);
    int replace(KEY* key, VAL* val);
    DictEntry<KEY, VAL>* findEntry(KEY* key);
    VAL* fetchValue(KEY* key);
    int deleteEntry(KEY* key);
    bool isRehashing() const;
    void rehashSteps(uint32_t steps);
    uint32_t dictScan(uint32_t cursor,
                      uint32_t steps,
                      void (*scanProc)(void* priv, KEY *key, VAL *val),
                      void *priv);
    uint32_t dictScanOneStep(uint32_t cursor,
                             void (*scanProc)(void* priv, KEY *key, VAL *val),
                             void *priv);
    int expand(uint32_t size);              // 扩容/缩容
    uint32_t size() const;
    
private:
    uint32_t nextPower(uint32_t num);
    uint32_t revBits(uint32_t bits);
    std::array<HashTable<KEY, VAL>*, 2> ht;
    int64_t rehashIndex = -1;

    AbstractLogHandler *logHandler;
};

#endif //FLYDB_DICT_H
