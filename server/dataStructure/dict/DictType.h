//
// Created by 赵立伟 on 2018/9/20.
//

#ifndef FLYDB_DICTTYPE_H
#define FLYDB_DICTTYPE_H

class DictType {
public:
    unsigned int (*hashFunction)(const void *key) = 0;
    void *(*keyDup)(const void *key) = 0;
    void *(*valDup)(const void *obj) = 0;
    int (*keyCompare)(const void *key1, const void *key2) = 0;
    void (*keyDestructor)(void *key) = 0;
    void (*valDestructor)(void *obj) = 0;
};

#endif //FLYDB_DICTTYPE_H
