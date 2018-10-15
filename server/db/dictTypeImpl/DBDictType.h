//
// Created by 赵立伟 on 2018/10/13.
//

#ifndef FLYDB_DBDICTTYPE_H
#define FLYDB_DBDICTTYPE_H

#include "../../dataStructure/dict/DictType.h"
#include "../../dataStructure/dict/Dict.h"
#include "../../dataStructure/flyObj/FlyObj.h"

class DBDictType : public DictType {
private:
    DBDictType() {
    }
public:
    static DictType* getInstance() {
        static DBDictType* instance;
        if (NULL == instance) {
            instance = new DBDictType();
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
        if (NULL == obj) {
            return;
        }

        FlyObj* flyObj = reinterpret_cast<FlyObj *>(obj);
        flyObj->decrRefCount();
    }
};

#endif //FLYDB_DBDICTTYPE_H
