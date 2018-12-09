//
// Created by 赵立伟 on 2018/9/24.
//

#include "SkipListNode.h"

SkipListNode::SkipListNode(const SkipListType* type, void* obj, double score) : type(type) {
    this->obj = obj;
    this->score = score;
    this->previous = NULL;
    for (int i = 0; i < SKIP_LIST_MAX_LEVEL; i++) {
        this->levels.push_back(SkipListLevel());
    }
}

std::vector<SkipListLevel> &SkipListNode::getLevels() {
    return levels;
}

bool SkipListNode::scoreLtRange(SkipListRange range) {
    return range.minex ? this->score <= range.min : this->score < range.min;
}

bool SkipListNode::scoreGtRange(SkipListRange range) {
    return range.maxex ? this->score >= range.max : this->score > range.max;
}

bool SkipListNode::scoreInRange(SkipListRange range) {
    if (!scoreLtRange(range) && !scoreGtRange(range)) {
        return true;
    }

    return false;
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

const SkipListType *SkipListNode::getType() const {
    return type;
}

void SkipListNode::setPrevious(SkipListNode *previous) {
    SkipListNode::previous = previous;
}

SkipListNode::~SkipListNode() {
    type->destructor(this->obj);
}
