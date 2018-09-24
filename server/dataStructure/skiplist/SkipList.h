//
// Created by 赵立伟 on 2018/9/24.
//

#ifndef FLYDB_SKIPLIST_H
#define FLYDB_SKIPLIST_H


#include "SkipListNode.h"

// 按小-->大的顺序排序
class SkipList {
public:
    SkipList(SkipListType& type);
    SkipListNode *getHeader() const;
    SkipListNode *getTailer() const;
    unsigned long getLength() const;
    int getLevel() const;
    int insertNode(double score, void* obj);
    unsigned int randomLevel();

private:
    SkipListNode *header, *tailer;
    unsigned long length;
    int level;
    SkipListType& type;
};

#endif //FLYDB_SKIPLIST_H
