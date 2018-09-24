//
// Created by 赵立伟 on 2018/9/24.
//

#ifndef FLYDB_SKIPLISTNODE_H
#define FLYDB_SKIPLISTNODE_H

#include <string>
#include <vector>
#include "SkipListType.h"

class SkipListNode;
struct SkipListLevel {
    SkipListLevel() {
    };

    SkipListLevel(unsigned int span, SkipListNode* next) {
        this->span = span;
        this->next = next;
    }
    unsigned int span;
    SkipListNode* next;
};

class SkipListNode {
public:
    SkipListNode(const SkipListType& type, void* obj, double score, unsigned int level);

private:
    void* obj;
    double score;
    std::vector<SkipListLevel> levels;
    SkipListNode *previous;
    const SkipListType& type;
};


#endif //FLYDB_SKIPLISTNODE_H
