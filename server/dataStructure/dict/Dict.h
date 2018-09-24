//
// Created by 赵立伟 on 2018/9/20.
//

#ifndef FLYDB_DICT_H
#define FLYDB_DICT_H

#include <map>
#include <array>
#include "HashTable.h"
#include "DictDef.h"

class Dict {
public:
    Dict(const DictType& type);
    virtual ~Dict();

    int addEntry(void* key, void* val);
    int replace(void* key, void* val);
    DictEntry* findEntry(void* key);
    void* fetchValue(void* key);
    int deleteEntry(void* key);
    bool isRehashing() const;
    void rehashSteps(int steps);
    unsigned long dictScan(unsigned long cursor, int steps, scanProc proc, void *priv);
    unsigned long dictScanOneStep(unsigned long cursor, scanProc proc, void *priv);
    
private:
    unsigned long revBits(unsigned long bits);
    std::array<class HashTable*, 2> ht;
    const DictType& type;
    int rehashIndex = -1;
};


#endif //FLYDB_DICT_H
