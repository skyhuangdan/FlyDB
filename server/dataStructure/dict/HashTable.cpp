//
// Created by 赵立伟 on 2018/9/20.
//
#include <iostream>

#include "HashTable.h"

const int REHASH_RATIO = 1;

HashTable::HashTable(DictType* type) : type(type) {
    this->table = new DictEntry*[4];
    this->size = HASH_TABLE_INITIAL_SIZE;
    this->used = 0;
}

int HashTable::addEntry(void* key, void* val) {
    int index = this->type->hashFunction(key);

    // 判断是否已经有相同的键，如果有，则不能继续插入
    if (hasSameKey(key)) {
        std::cout << "have same key in ht!" << key << std::endl;
        return -1;
    }

    // 将该entry插入头部
    DictEntry* entry = new DictEntry(key, val);
    struct DictEntry* head = this->table[index];
    this->table[index] = head;
    entry->next = head;

    this->size++;
    return 1;
}

DictEntry* HashTable::findEntry(void* key) {
    int index = this->type->hashFunction(key);
    struct DictEntry* node = this->table[index];
    while (node != NULL) {
        if (this->type->keyCompare(node->key, key) > 0) {
           return node;
        }
        node = node->next;
    }
    return NULL;
}

bool HashTable::hasSameKey(void* key) {
    return findEntry(key) != NULL;
}

bool HashTable::needExpand() {
    return this->size * REHASH_RATIO >= this->used;
}