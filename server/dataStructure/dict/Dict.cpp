//
// Created by 赵立伟 on 2018/9/20.
//

#include <iostream>
#include "Dict.h"

Dict::Dict(DictType* const type) : type(type) {
    this->ht[0] = new HashTable(type, HASH_TABLE_INITIAL_SIZE);
    this->ht[1] = new HashTable(type, HASH_TABLE_INITIAL_SIZE);
    this->rehashIndex = -1;
}

bool Dict::isRehashing() {
    return this->rehashIndex >= 0;
}

int Dict::addEntry(void* key, void* val) {
    if (NULL == key || NULL == val) {
        std::cout << "key or value is NULL, key = "
                     << key << "value = " << val << std::endl;
        return -1;
    }

    HashTable* ht = this->ht[0];
    if (isRehashing()) {
        // 进行一步rehash
        rehashOneStep(1);
        ht = this->ht[1];
    } else {
        // todo: 判断是否需要扩容
    }

    return ht->addEntry(key, val);
}

DictEntry* Dict::findEntry(void* key) {
    if (NULL == key) {
        return NULL;
    }

    // 先进行一步rehash
    if (isRehashing()) {
        rehashOneStep(1);
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
        rehashOneStep(1);
    }

    DictEntry* entry = findEntry(key);
    return NULL == entry ? NULL : entry->val;
}

int Dict::deleteEntry(void* key) {
    if (NULL == key) {
        std::cout << "key or value is NULL, key = " << key << std::endl;
        return NULL;
    }

    // 先进行一步rehash
    if (isRehashing()) {
        rehashOneStep(1);
    }

    // 如果从ht[0]中删除成功，返回
    HashTable* ht = this->ht[0];
    int tmp = ht->deleteEntry(key);
    if (tmp > 0) {
        return tmp;
    }

    // 如果ht[0]中删除失败，并且处于rehash过程中，则从ht[1]中尝试删除
    if (isRehashing()) {
        return this->ht[1]->deleteEntry(key);
    }

    return 0;
}

int Dict::replace(void* key, void* val) {
    if (NULL == key || NULL == val) {
        std::cout << "key or value is NULL, key = "
                     << key << "value = " << val << std::endl;
        return -1;
    }

    // 先进行一步rehash
    if (isRehashing()) {
        rehashOneStep(1);
    }

    // 先执行插入步骤，如果插入成功，说明之前没有该key，直接返回成功
    if(addEntry(key, val) > 0) {
        return 1;
    }

    // 如果插入失败，说明之前存在该key，需要替换value
    DictEntry* entry = findEntry(key);
    entry->val = this->type->valDup(val);
    return 1;
}

void Dict::rehashOneStep(int steps) {
    return;
}