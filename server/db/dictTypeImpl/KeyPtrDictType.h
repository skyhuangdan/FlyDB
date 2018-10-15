//
// Created by 赵立伟 on 2018/10/14.
//

#ifndef FLYDB_KEYPTRDICTTYPE_H
#define FLYDB_KEYPTRDICTTYPE_H


#include "../../dataStructure/dict/Dict.h"

class KeyPtrDictType : public DictType {
private:
    KeyPtrDictType() {
    }

public:
    static DictType* getInstance() {
        static KeyPtrDictType* instance;
        if (NULL == instance) {
            instance = new KeyPtrDictType();
        }
        return instance;
    }

    uint64_t hashFunction(const void *key) const {
        return dictStrHash(key);
    };

    int keyCompare(const void *key1, const void *key2) const {
        return dictStrKeyCompare(key1, key2);
    }
};


#endif //FLYDB_KEYPTRDICTTYPE_H
