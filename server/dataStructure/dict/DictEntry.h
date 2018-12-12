//
// Created by 赵立伟 on 2018/9/20.
//

#ifndef FLYDB_DICTENTRY_H
#define FLYDB_DICTENTRY_H

#include <string>

template<class KEY, class VAL>
struct DictEntry {
 public:
    DictEntry(KEY *key, VAL *val);
    virtual ~DictEntry();
    KEY *getKey() const;
    VAL *getVal() const;
    void setVal(void *val);

    KEY* key;
    VAL* val;
    DictEntry* next;

};


#endif //FLYDB_DICTENTRY_H
