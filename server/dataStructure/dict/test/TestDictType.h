//
// Created by 赵立伟 on 2018/9/21.
//

#ifndef FLYDB_TESTDICTTYPE_H
#define FLYDB_TESTDICTTYPE_H

#include "../DictType.h"

void testScanProc(void* priv, void* key, void* val);

class TestDictType : public DictType {
public:
    unsigned long hashFunction(const void *key) const;
    void* keyDup(const void *key) const;
    void* valDup(const void *obj) const;
    int keyCompare(const void *key1, const void *key2) const ;
    void keyDestructor(void *key) const;
    void valDestructor(void *obj) const;
};

#endif //FLYDB_TESTDICTTYPE_H
