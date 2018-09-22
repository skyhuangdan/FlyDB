//
// Created by 赵立伟 on 2018/9/20.
//

#include <iostream>
#include "Dict.h"

Dict::Dict(DictType* const type) : type(type) {
    this->ht[0] = new HashTable(type, HASH_TABLE_INITIAL_SIZE);
    this->ht[1] = NULL;
    this->rehashIndex = -1;
}

bool Dict::isRehashing() const {
    return this->rehashIndex >= 0;
}

int Dict::addEntry(void* key, void* val) {
    if (NULL == key || NULL == val) {
        std::cout << "key or value is NULL, key = "
                     << key << "value = " << val << std::endl;
        return -1;
    }

    int res = 0;
    if (isRehashing()) {  // 如果正在rehash
        // 进行一步rehash
        rehashSteps(1);
        // 插入操作
        res = this->ht[1]->addEntry(key, val);
    } else {  // 如果没在rehash, 执行插入操作；并判断是否需要扩容
        if ((res = this->ht[0]->addEntry(key, val)) > 0) {
            if (this->ht[0]->needExpand()) {
                this->ht[1] = new HashTable(this->type, this->ht[0]->getSize() * 2);
                this->rehashIndex = 0;
                rehashSteps(1);
            }
        }
    }

    // 如果插入成功，判断是否需要rehash
    return res;
}

DictEntry* Dict::findEntry(void* key) {
    if (NULL == key) {
        return NULL;
    }

    // 先进行一步rehash
    if (isRehashing()) {
        rehashSteps(1);
    }

    // 先查找ht[0]
    HashTable* ht = this->ht[0];
    DictEntry* entry = ht->findEntry(key);

    // 如果ht[0]中没有找到，并且正在进行rehash，则查找ht[1]
    if (NULL == entry && isRehashing()) {
        ht = this->ht[1];
        entry = ht->findEntry(key);
    }

    return entry;
}

void* Dict::fetchValue(void* key) {
    if (NULL == key) {
        std::cout << "key or value is NULL, key = " << key << std::endl;
        return NULL;
    }

    // 先进行一步rehash
    if (isRehashing()) {
        rehashSteps(1);
    }

    DictEntry* entry = findEntry(key);
    return NULL == entry ? NULL : entry->val;
}

int Dict::deleteEntry(void* key) {
    if (NULL == key) {
        std::cout << "key or value is NULL, key = " << key << std::endl;
        return -1;
    }

    // 先进行一步rehash
    if (isRehashing()) {
        rehashSteps(1);
    }

    // 如果从ht[0]中删除成功，返回
    HashTable* ht = this->ht[0];
    int tmp = ht->deleteEntry(key);
    if (tmp > 0) {
        if (ht->needShrink()) {
            this->ht[1] = new HashTable(this->type, this->ht[0]->getShrinkSize());
            this->rehashIndex = 0;
            rehashSteps(1);
        }
        return tmp;
    }

    // 如果ht[0]中删除失败，并且处于rehash过程中，则从ht[1]中尝试删除
    if (isRehashing()) {
        return this->ht[1]->deleteEntry(key);
    }

    return 1;
}

int Dict::replace(void* key, void* val) {
    if (NULL == key || NULL == val) {
        std::cout << "key or value is NULL, key = "
                     << key << "value = " << val << std::endl;
        return -1;
    }

    // 先进行一步rehash
    if (isRehashing()) {
        rehashSteps(1);
    }

    // 先执行插入步骤，如果插入成功，说明之前没有该key，直接返回成功
    if (addEntry(key, val) > 0) {
        return 1;
    }

    // 如果插入失败，说明之前存在该key，需要替换value
    DictEntry* entry = findEntry(key);
    entry->val = this->type->valDup(val);
    return 1;
}


void Dict::rehashSteps(int steps) {
    for (int i = 0; i < steps && !this->ht[0]->isEmpty(); i++) {
        // 找到存在元素的index
        DictEntry* entry = NULL;
        while (NULL == entry && this->rehashIndex < this->ht[0]->getSize()) {
            entry = this->ht[0]->getEntryBy(this->rehashIndex++);
        }

        // 对该index下的链表中所有元素进行迁移
        while (entry != NULL) {
            this->ht[1]->addEntry(entry->getKey(), entry->getVal());
            this->ht[0]->deleteEntry(entry->getKey());
            entry = entry->next;
        }
    }

    // 已经rehash完毕
    if (this->ht[0]->isEmpty()) {
        delete this->ht[0];
        this->ht[0] = this->ht[1];
        this->ht[1] = NULL;
        this->rehashIndex = -1;
    }

    return;
}

Dict::~Dict() {
    delete this->ht[0];
    if (isRehashing()) {
        delete this->ht[1];
    }
}
