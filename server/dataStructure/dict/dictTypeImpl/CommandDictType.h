//
// Created by 赵立伟 on 2018/10/13.
//

#ifndef FLYDB_DBDICTTYPE_H
#define FLYDB_DBDICTTYPE_H

#include "../DictType.h"

class CommandDictType : public DictType {
public:
    static DictType* getInstance() {
        static CommandDictType* instance;
        if (NULL == instance) {
            instance = new CommandDictType();
        }
        return instance;
    }

    uint64_t hashFunction(const void *key) const {
        return dictStrHash(key);
    }

    int keyCompare(const void *key1, const void *key2) const {
        return dictStrKeyCompare(key1, key2);
    }

    void keyDestructor(void *key) const {
        return dictStrDestructor(key);
    }

    void valDestructor(void *obj) const {
    }

private:
    CommandDictType() {
    }
};

#endif //FLYDB_DBDICTTYPE_H
