//
// Created by 赵立伟 on 2018/9/20.
//

#ifndef FLYDB_DICT_H
#define FLYDB_DICT_H

#include <map>
#include <array>
#include "HashTable.h"


class Dict {
public:
    Dict(class DictType* type);
    int addEntry(void* key, void* val);
    int replace(void* key, void* val);
    DictEntry* findEntry(void* key);
    void* fetchValue(void* key);
    bool isRehashing();
    void rehashStep(int steps);
private:
    std::array<class HashTable*, 2> ht;
    class DictType* type;
    int rehashIndex;
};


#endif //FLYDB_DICT_H
