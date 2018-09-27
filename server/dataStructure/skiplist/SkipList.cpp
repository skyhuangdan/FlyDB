//
// Created by 赵立伟 on 2018/9/24.
//

#include <random>
#include "SkipList.h"
#include "SkipListDef.h"

SkipList::SkipList(const SkipListType& type) : type(type) {
    this->level = 1;
    this->length = 0;
    this->header = this->tailer = new SkipListNode(type, NULL, 0);
}

SkipListNode *SkipList::getHeader() const {
    return header;
}

SkipListNode *SkipList::getTailer() const {
    return tailer;
}

uint32_t SkipList::getLength() const {
    return length;
}

uint32_t SkipList::getLevel() const {
    return level;
}

void SkipList::insertNode(double score, void* obj) {
    std::vector<SkipListNode*> forwards(this->level);
    std::vector<uint32_t> rank(this->level + 1, 0); // 第level+1个作为哨兵

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
        forwards[i] = temp;
    }

    // 假如level > 跳跃表当前level, 初始化新增加的几个层，用于后面计算
    uint8_t randLevel = randomLevel();
    for (uint32_t i = this->level; i < randLevel; i++) {
        forwards[i] = header;
        rank[i] = 0;
        header->getLevels()[i].span = this->length;
    }

    // 设置tailer以及previous指针
    SkipListNode *nodeToInsert = new SkipListNode(this->type, obj, score);
    nodeToInsert->setPrevious(forwards[0] == header ? NULL : forwards[0]);
    if (NULL == forwards[0]->getLevels()[0].next) {
        this->tailer = nodeToInsert;
    } else {
        forwards[0]->getLevels()[0].next->setPrevious(nodeToInsert);
    }

    // 实际的插入操作，并计算相应节点的span
    for (uint32_t i = 0; i < randLevel; i++) {
        nodeToInsert->getLevels()[i].next = forwards[i]->getLevels()[i].next;
        forwards[i]->getLevels()[i].next = nodeToInsert;
        nodeToInsert->getLevels()[i].span = forwards[i]->getLevels()[i].span - (rank[0] - rank[i]);
        forwards[i]->getLevels()[i].span = rank[0] - rank[i] + 1;
    }

    // 如果level小于跳跃表的level，将没有进行插入的level的前节点span+1
    for (uint32_t i = randLevel; i < this->level; i++) {
        forwards[i]->getLevels()[i].span += 1;
    }

    this->level = randLevel > this->level ? randLevel : this->level;
    this->length++;
}

/**
 * 以SKIP_LIST_MAX_LEVEL = 4为例，maxNum = 8
 * rand:    1      2       3       4       5       6       7       8
 * 概率：   1/4    1/4     |-> 1/2 <-|     |--------> 1/2 <----------|
 * level:   4      3           2                      1
 */
uint8_t SkipList::randomLevel() {
    uint32_t maxNum = 1 << (SKIP_LIST_MAX_LEVEL - 1);

    // 生成随机数
    std::default_random_engine engine;
    std::uniform_int_distribution<uint32_t> distribution(1, maxNum);
    uint32_t rand = distribution(engine);

    // 根据随机数获取具体的level
    uint8_t power = 0;
    for (uint32_t num = maxNum; num >= 1; num = num >> 1) {
        if (rand > num) {
            break;
        }
        power++;
    }
    return power;
}

int SkipList::deleteNode(double score, void* obj, SkipListNode** res) {
    std::vector<SkipListNode*> forwards(this->level);
    for (int l = this->level - 1; l >= 0; l--) {
        SkipListNode* prevNode = this->header;
        SkipListNode* node = prevNode->getLevels()[l].next;
        // 找到node==NULL或者score、obj都相等的node
        while (node != NULL && (node->getScore() != score || 0 != type.compare(node->getObj(), obj))) {
            prevNode = node;
            node = prevNode->getLevels()[l].next;
        }
        forwards[l] = prevNode;
    }

    if (NULL == forwards[0]->getLevels()[0].next) {  // 没有找到符合条件的node
        return -1;
    } else {  // 处理previous指针
        SkipListNode* node = *res = forwards[0]->getLevels()[0].next;
        if (NULL == node->getLevels()[0].next) {
            this->tailer = forwards[0];
        } else {
            node->getLevels()[0].next->setPrevious(forwards[0]);
        }
    }

    for (int i = 0; i < level; i++) {
        SkipListNode* node = forwards[i]->getLevels()[i].next;
        // 如果从该层开始找不到待删除节点，上面的那些层也不必找了
        if (NULL == node) {
            break;
        }

        // 该层只剩header和待删除节点，那么level-1
        if (forwards[i] == this->header && NULL == node->getLevels()[i].next) {
            this->level--;
        }

        forwards[i]->getLevels()[i].next = node->getLevels()[i].next;
        forwards[i]->getLevels()[i].span += node->getLevels()[i].span - 1;
    }

    this->length--;
    return 0;
}

uint32_t SkipList::getRank(double score, void* obj) {
    int totalSpan = 0;

    SkipListNode* prev = this->header;
    for (int l = this->level - 1; l >= 0; l--) {
        SkipListNode* next = prev->getLevels()[l].next;

        /**
          * 如果next小的话，往后遍历。该循环停下来有三个条件：
          *      1.next==NULL
          *      2.找到合适的node
          *      3.当前node超过了查找的值，此时需要向下一层寻找
         **/
        while (next != NULL && (next->getScore() < score || type.compare(next->getObj(), obj) < 0)) {
            prev = next;
            next = prev->getLevels()[l].next;
            totalSpan += prev->getLevels()[l].span;
        }

        // 如果找到了，直接返回
        if (next != NULL && next->getScore() == score && type.compare(next->getObj(), obj) == 0) {
            totalSpan += prev->getLevels()[l].span;
            return totalSpan;
        }
    }

    return -1;
}

SkipListNode* SkipList::getNodeByRank(uint32_t rank) {
    SkipListNode* node = this->header;

    for (int l = this->level - 1; l >= 0; l--) {
        while (node != NULL && node->getLevels()[l].span < rank) {
            rank -= node->getLevels()[l].span;
            node = node->getLevels()[l].next;
        }

        if (NULL == node) {
            return NULL;
        }

        if (node->getLevels()[l].span == rank) {
            return node->getLevels()[l].next;
        }
    }

    return NULL;
}

SkipList::~SkipList() {
    delete header;
}
