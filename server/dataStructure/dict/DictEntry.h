//
// Created by 赵立伟 on 2018/9/20.
//

#ifndef FLYDB_DICTENTRY_H
#define FLYDB_DICTENTRY_H

#include <string>

struct DictEntry {
 public:
    DictEntry(void* key, void* val) {
        this->key = key;
        this->val = val;
    };

    void* key;
    void* val;
    struct DictEntry* next;
};


#endif //FLYDB_DICTENTRY_H
