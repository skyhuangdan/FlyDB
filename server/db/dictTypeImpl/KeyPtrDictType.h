//
// Created by 赵立伟 on 2018/10/14.
//

#ifndef FLYDB_KEYPTRDICTTYPE_H
#define FLYDB_KEYPTRDICTTYPE_H


#include "../../dataStructure/dict/Dict.h"

class KeyPtrDictType : public DictType {
public:
    uint64_t hashFunction(const void *key) const {
        return dictStrHash(key);
    };

    int keyCompare(const void *key1, const void *key2) const {
        return dictStrKeyCompare(key1, key2);
    }
};


#endif //FLYDB_KEYPTRDICTTYPE_H
