//
// Created by 赵立伟 on 2018/9/24.
//

#ifndef FLYDB_SKIPLIST_H
#define FLYDB_SKIPLIST_H
#include "SkipListNode.h"

// 按小-->大的顺序排序
class SkipList {
public:
    SkipList(const SkipListType& type);
    virtual ~SkipList();
    SkipListNode *getHeader() const;
    SkipListNode *getTailer() const;
    uint32_t getLength() const;
    uint32_t getLevel() const;
    void insertNode(double score, void* obj);
    int deleteNode(double score, void* obj, SkipListNode** res);
    uint32_t getRank(double score, void* obj);
    SkipListNode* getNodeByRank(uint32_t rank);
    uint8_t randomLevel();

private:
    SkipListNode *header, *tailer;
    uint32_t length;
    uint32_t level;
    const SkipListType& type;
};

#endif //FLYDB_SKIPLIST_H
