//
// Created by 赵立伟 on 2018/9/24.
//

#include "SkipListNode.h"

SkipListNode::SkipListNode(const SkipListType& type, void* obj, double score, uint8_t level) : type(type) {
    this->obj = obj;
    this->score = score;
    for (int i = 0; i < level; i++) {
        this->levels.push_back(SkipListLevel());
    }
}

std::vector<SkipListLevel> &SkipListNode::getLevels() {
    return levels;
}

void *SkipListNode::getObj() const {
    return obj;
}

double SkipListNode::getScore() const {
    return score;
}

SkipListNode *SkipListNode::getPrevious() const {
    return previous;
}

const SkipListType &SkipListNode::getType() const {
    return type;
}

void SkipListNode::setPrevious(SkipListNode *previous) {
    SkipListNode::previous = previous;
}

SkipListNode::~SkipListNode() {
    type.destructor(this->obj);
    delete levels[0].next;
}
