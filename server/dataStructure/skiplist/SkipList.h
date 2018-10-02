//
// Created by 赵立伟 on 2018/9/24.
//

#ifndef FLYDB_SKIPLIST_H
#define FLYDB_SKIPLIST_H
#include "SkipListNode.h"
#include "SkipListDef.h"

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
    int isInRange(SkipListRange range);
    SkipListNode* firstInRange(SkipListRange range);
    SkipListNode* lastInRange(SkipListRange range);
    uint32_t deleteRangeByScore(SkipListRange range);
    uint32_t deleteRangeByRank(uint32_t start, uint32_t end);
    int deleteNode(SkipListNode* node); // 会释放节点

private:
    uint8_t randomLevel();
    SkipListNode *header, *tailer;
    uint32_t length;
    uint8_t level;
    const SkipListType& type;
};

#endif //FLYDB_SKIPLIST_H
