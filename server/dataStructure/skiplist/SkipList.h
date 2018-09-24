//
// Created by 赵立伟 on 2018/9/24.
//

#ifndef FLYDB_SKIPLIST_H
#define FLYDB_SKIPLIST_H


#include "SkipListNode.h"

class SkipList {
public:
    SkipList(SkipListType& type);
    SkipListNode *getHeader() const;
    SkipListNode *getTailer() const;
    unsigned long getLength() const;
    int getLevel() const;

private:
    SkipListNode *header, *tailer;
    unsigned long length;
    int level;
    SkipListType& type;
};

#endif //FLYDB_SKIPLIST_H
