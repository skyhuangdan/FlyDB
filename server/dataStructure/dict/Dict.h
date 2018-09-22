//
// Created by 赵立伟 on 2018/9/20.
//

#ifndef FLYDB_DICT_H
#define FLYDB_DICT_H

#include <map>
#include <array>
#include "HashTable.h"

const int HASH_TABLE_INITIAL_SIZE = 4;

class Dict {
public:
    Dict(DictType* const type);
    virtual ~Dict();

    int addEntry(void* key, void* val);
    int replace(void* key, void* val);
    DictEntry* findEntry(void* key);
    void* fetchValue(void* key);
    int deleteEntry(void* key);
    bool isRehashing() const;
    void rehashSteps(int steps);
    unsigned int getShrinkSize(unsigned int size, unsigned int used) const;

private:
    std::array<class HashTable*, 2> ht;
    DictType* const type;
    int rehashIndex = -1;
};


#endif //FLYDB_DICT_H
