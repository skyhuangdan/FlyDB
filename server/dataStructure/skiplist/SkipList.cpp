//
// Created by 赵立伟 on 2018/9/24.
//

#include "SkipList.h"
#include "SkipDef.h"

SkipList::SkipList(SkipListType& type) : type(type) {
    this->level = 1;
    this->length = 0;
    this->header = this->tailer = new SkipListNode(type, NULL, 0, SKIP_LIST_MAX_LEVEL);
}

SkipListNode *SkipList::getHeader() const {
    return header;
}

SkipListNode *SkipList::getTailer() const {
    return tailer;
}

unsigned long SkipList::getLength() const {
    return length;
}

int SkipList::getLevel() const {
    return level;
}
