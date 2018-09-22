//
// Created by 赵立伟 on 2018/9/20.
//

#ifndef FLYDB_DICTENTRY_H
#define FLYDB_DICTENTRY_H

#include <string>
#include "dictDef.h"

struct DictEntry {
 public:
    DictEntry(void* key, void* val, DictType* const type);
    virtual ~DictEntry();
    void *getKey() const;
    void *getVal() const;
    void setVal(void *val);

    void* key;
    void* val;
    DictEntry* next;
    DictType* const type;

};


#endif //FLYDB_DICTENTRY_H
