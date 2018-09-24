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

int SkipList::insertNode(double score, void* obj) {
    std::vector<SkipListNode*> forwards;
    std::vector<unsigned int> rank(level+1, 0); // 第level+1个作为哨兵

    /*
     * **是待插入点node， 插入前节点是prevNode[level]
     * | ============================================> |    level = 3
     * | =================================**=> | ====> |    level = 2
     * | ====================> | =========**=> | ====> |    level = 1
     * | ====> | ====> | ====> | ====> | =**=> | ====> |    level = 0
     * a0                      a1      a2      a3
     * 插入前: rank[0] = a2, rank[1] = a1, rank[2] = a0
     * 插入后: prevNode[1].span = a2 - a1 + 1 = rank[0] - rank[1] + 1
     *        prevNode[2].span = a2 - a0 + 1 = rank[0] - rank[2] + 1
     *        ====> peveNode[i].span = rank[0] - rank[i] + 1
     *        ====> node[level] = 原prevNode[level].span - prevNode[level].span + 1
     */
    // forwards代表待插入点的前一个节点, rank则代表该节点在跳跃表中的排序
    SkipListNode* temp = this->header;
    for (int i = this->level-1; i >= 0; i--) {
        rank[i] = rank[i+1];                  // 哨兵用于这里
        SkipListNode* next = temp->getLevels()[i].next;
        while (NULL != next && (next->getScore() < score
                   || (next->getScore() == score && (this->type.compare(next->getObj(), obj) < 0)))) {
            rank[i] += temp->getLevels()[i].span;
            temp = next;
            next = next->getLevels()[i].next;
        }
        forwards[i] = next;
    }

    // 假如level > 跳跃表当前level, 初始化新增加的几个层，用于后面计算
    for (int i = this->level; i < level; i) {
        forwards[i] = header;
        rank[i] = 0;
        header->getLevels()[i].span = this->length;
    }

    // 实际的插入操作，并计算相应节点的span
    unsigned int level = randomLevel();
    SkipListNode *nodeToInsert = new SkipListNode(this->type, obj, score, level);
    for (int i = 0; i < level; i++) {
        nodeToInsert->getLevels()[i].next = forwards[i]->getLevels()[i].next;
        forwards[i]->getLevels()[i].next = nodeToInsert;
        nodeToInsert->getLevels()[i].span = forwards[i]->getLevels()[i].span - (rank[0] - rank[i]);
        forwards[i]->getLevels()[i].span = rank[0] - rank[i] + 1;
    }
    nodeToInsert->setPrevious(forwards[0] == header ? NULL : forwards[0]);
    if (NULL == forwards[0]->getLevels()[0].next) {
        this->tailer = nodeToInsert;
    } else {
        forwards[0]->getLevels()[0].next->setPrevious(nodeToInsert);
    }

    // 如果level小于跳跃表的level，将没有进行插入的level的前节点span+1
    for (int i = level; i < this->level; i++) {
        forwards[i]->getLevels()[i].span += 1;
    }

    this->level = level > this->level ? level : this->level;
    this->length++;
}

unsigned int SkipList::randomLevel() {
    uint64_t rand = random() % (2 << SKIP_LIST_MAX_LEVEL) + 1;
    int order = 0;
    while (rand >= (2 << order++));

    return 2 << (order-1);
}
