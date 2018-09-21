//
// Created by 赵立伟 on 2018/9/20.
//
#include <iostream>

#include "HashTable.h"

const int REHASH_RATIO = 1;

HashTable::HashTable(DictType* const type, int size) : type(type), size(size) {
    this->table = new DictEntry*[size];
    for (int i = 0; i < size; i++) {
        this->table[i] = NULL;
    }
    this->used = 0;
    this->mask = size - 1;
}

int HashTable::getIndex(void* key) {
    return this->type->hashFunction(key) & this->mask;
}

int HashTable::addEntry(void* key, void* val) {
    int index = getIndex(key);

    // 判断是否已经有相同的键，如果有，则不能继续插入
    if (hasKey(key)) {
        std::cout << "have same key in ht!" << key << std::endl;
        return -1;
    }

    // 将该entry插入头部
    DictEntry* entry = new DictEntry(key, val, this->type);
    DictEntry* head = this->table[index];
    this->table[index] = head;
    entry->next = head;

    this->size++;
    return 1;
}

DictEntry* HashTable::findEntry(void* key) {
    int index = getIndex(key);
    DictEntry* node = this->table[index];
    while (node != NULL) {
        if (this->type->keyCompare(node->key, key) > 0) {
           return node;
        }
        node = node->next;
    }
    return NULL;
}

int HashTable::deleteEntry(void* key) {
    int index = getIndex(key);
    DictEntry* node = this->table[index];
    if (node != NULL) {
        // 如果要删除的key是头结点
        if (this->type->keyCompare(node, key)) {
            free(node);
            node = NULL;
            return 1;
        }

        // 如果不是头结点，则查找链表中是否有该节点
        while(node->next != NULL) {
            if (this->type->keyCompare(node->key, key)) {
                DictEntry* tmp = node->next;
                node->next = node->next->next;
                free(tmp);
                return 1;
            }
            node = node->next;
        }
    }

    return 0;
}

bool HashTable::hasKey(void* key) {
    return findEntry(key) != NULL;
}

bool HashTable::needExpand() {
    return this->size * REHASH_RATIO >= this->used;
}