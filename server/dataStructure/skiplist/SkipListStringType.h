//
// Created by 赵立伟 on 2018/12/9.
//

#ifndef FLYDB_SKIPLISTSTRINGTYPE_H
#define FLYDB_SKIPLISTSTRINGTYPE_H

#include "SkipListType.h"

class SkipListStringType : public SkipListType {
public:
    static SkipListType *getInstance();
    int compare(const void* val1, const void* val2) const;
    void* dup(const void* val) const;
    void destructor(void *obj) const;
private:
    SkipListStringType() {};
};


#endif //FLYDB_SKIPLISTSTRINGTYPE_H
