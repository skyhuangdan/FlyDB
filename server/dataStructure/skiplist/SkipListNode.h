//
// Created by 赵立伟 on 2018/9/24.
//

#ifndef FLYDB_SKIPLISTNODE_H
#define FLYDB_SKIPLISTNODE_H

#include <string>
#include <vector>
#include "SkipListType.h"
#include "SkipListDef.h"

class SkipListNode;
struct SkipListLevel {
    SkipListLevel() {
        this->span = 0;
        this->next = NULL;
    };

    uint32_t span;
    SkipListNode* next;
};

class SkipListNode {
public:
    SkipListNode(const SkipListType& type, void* obj, double score);
    virtual ~SkipListNode();
    void *getObj() const;
    double getScore() const;
    SkipListNode *getPrevious() const;
    void setPrevious(SkipListNode *previous);
    const SkipListType &getType() const;
    std::vector<SkipListLevel> &getLevels();
    bool scoreLtRange(SkipListRange range);
    bool scoreGtRange(SkipListRange range);
    bool scoreInRange(SkipListRange range);

private:
    void* obj;
    double score;
    std::vector<SkipListLevel> levels;
    SkipListNode *previous;
    const SkipListType& type;
};


#endif //FLYDB_SKIPLISTNODE_H
