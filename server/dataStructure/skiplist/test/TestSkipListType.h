//
// Created by 赵立伟 on 2018/9/26.
//

#ifndef FLYDB_TESTSKIPLISTTYPE_H
#define FLYDB_TESTSKIPLISTTYPE_H

#include "../SkipListType.h"

class TestSkipListType : public SkipListType {
public:
    /* 返回值：
     * < 0: val1 < val2
     * = 0: val1 = val2
     * > 0: val1 > val2
     */
    int compare(const void* val1, const void* val2) const;
    void* dup(const void* val) const;
    void destructor(void *obj) const;
};


#endif //FLYDB_TESTSKIPLISTTYPE_H
