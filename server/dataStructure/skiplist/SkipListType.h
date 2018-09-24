//
// Created by 赵立伟 on 2018/9/24.
//

#ifndef FLYDB_SKIPLISTTYPE_H
#define FLYDB_SKIPLISTTYPE_H
class SkipListType {
public:
    /* 返回值：
     * < 0: val1 < val2
     * = 0: val1 = val2
     * > 0: val1 > val2
     */
    virtual int compare(const void* val1, const void* val2) const = 0;
    virtual void* dup(const void* val) const = 0;
};

#endif //FLYDB_SKIPLISTTYPE_H
