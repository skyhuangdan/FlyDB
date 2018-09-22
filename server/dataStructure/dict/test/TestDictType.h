//
// Created by 赵立伟 on 2018/9/21.
//

#ifndef FLYDB_TESTDICTTYPE_H
#define FLYDB_TESTDICTTYPE_H

#include "../dictDef.h"

void testScanProc(void* priv, void* key, void* val);

class TestDictType : public DictType {
public:
    unsigned long hashFunction(const void *key);
    void* keyDup(const void *key);
    void* valDup(const void *obj);
    int keyCompare(const void *key1, const void *key2);
    void keyDestructor(void *key);
    void valDestructor(void *obj);
};

#endif //FLYDB_TESTDICTTYPE_H
