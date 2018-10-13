//
// Created by 赵立伟 on 2018/9/21.
//

#ifndef FLYDB_TESTDICTTYPE_H
#define FLYDB_TESTDICTTYPE_H

#include "../DictType.h"

void testScanProc(void* priv, void* key, void* val);

class TestDictType : public DictType {
public:
    uint64_t hashFunction(const void *key) const;
    void* keyDup(void *key) const;
    void* valDup(void *obj) const;
    int keyCompare(const void *key1, const void *key2) const ;
    void keyDestructor(void *key) const;
    void valDestructor(void *obj) const;
};

#endif //FLYDB_TESTDICTTYPE_H
