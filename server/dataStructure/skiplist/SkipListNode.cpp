//
// Created by 赵立伟 on 2018/9/24.
//

#include "SkipListNode.h"

SkipListNode::SkipListNode(const SkipListType& type, void* obj, double score, unsigned int level) : type(type) {
    this->obj = obj;
    this->score = score;
    for (int i = 0; i < level; i++) {
        this->levels.push_back(SkipListLevel());
    }
}
