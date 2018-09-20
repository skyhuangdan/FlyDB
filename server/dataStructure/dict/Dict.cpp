//
// Created by 赵立伟 on 2018/9/20.
//

#include <iostream>
#include "Dict.h"

Dict::Dict(class DictType* type) : type(type) {
    this->ht[0] = new HashTable(type);
    this->ht[1] = new HashTable(type);
    this->rehashing = false;
}

bool Dict::isRehashing() {
    return this->rehashing;
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
        rehashStep(1);
        ht = this->ht[1];
    }

    return ht->addEntry(key, val);
}

DictEntry* Dict::findEntry(void* key) {
    if (NULL == key) {
        return NULL;
    }

    HashTable* ht = this->ht[0];
    if (isRehashing()) {
        // 进行一步rehash
        rehashStep(1);
        ht = this->ht[1];
    }

    return ht->findEntry(key);
}

void* Dict::fetchValue(void* key) {
    if (NULL == key) {
        std::cout << "key or value is NULL, key = " << key << std::endl;
        return NULL;
    }
    DictEntry* entry = findEntry(key);
    return NULL == entry ? NULL : entry->val;
}

int Dict::replace(void* key, void* val) {
    if (NULL == key || NULL == val) {
        std::cout << "key or value is NULL, key = "
                     << key << "value = " << val << std::endl;
        return -1;
    }

    // 先执行插入步骤，如果插入成功，说明之前没有该key，直接返回成功
    if(addEntry(key, val) > 0) {
        return 1;
    }

    // 如果插入失败，说明之前存在该key，需要替换value
    DictEntry* entry = findEntry(key);
    entry->val = val;
    return 1;
}

void Dict::rehashStep(int steps) {
    return;
}