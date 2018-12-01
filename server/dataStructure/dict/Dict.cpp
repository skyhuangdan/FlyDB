//
// Created by 赵立伟 on 2018/9/20.
//

#include <iostream>
#include "Dict.h"
#include "../../utils/MiscTool.h"

Dict::Dict(const DictType* type) : type(type) {
    this->ht[0] = new HashTable(type, HASH_TABLE_INITIAL_SIZE);
    this->ht[1] = NULL;
    this->rehashIndex = -1;
}

Dict::~Dict() {
    delete this->ht[0];
    if (isRehashing()) {
        delete this->ht[1];
    }
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

    // 先从ht[0]中尝试删除, 如果删除成功
    if ((this->ht[0]->deleteEntry(key)) > 0) {
        // 没有处于rehash过程中 && 需要缩容，则进行缩容
        if (!isRehashing() && this->ht[0]->needShrink()) {
            this->ht[1] = new HashTable(this->type, this->ht[0]->getSize() / 2);
            this->rehashIndex = 0;
            rehashSteps(1);
        }
        return 1;
    } else if (isRehashing()) {  // 如果ht[0]中删除失败，并且正在rehash过程中, 则需要从ht[1]中进行查找删除
        return this->ht[1]->deleteEntry(key);
    }

    return -1;
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

void Dict::rehashSteps(uint32_t steps) {
    for (uint32_t i = 0; i < steps && !this->ht[0]->isEmpty(); i++) {
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

uint32_t Dict::dictScan(uint32_t cursor, uint32_t steps, scanProc proc, void *priv) {
    uint32_t nextCursor = cursor;
    for (uint32_t i = 0; i < steps; i++) {
        nextCursor = dictScanOneStep(nextCursor, proc, priv);
        if (0 == nextCursor) {
            return nextCursor;
        }
    }

    return nextCursor;
}

uint32_t Dict::dictScanOneStep(uint32_t cursor, scanProc proc, void *priv) {
    HashTable* ht0 = this->ht[0];
    HashTable* ht1 = this->ht[1];
    if (isRehashing()) {
        // 确保ht0保存小的hashtable
        if (ht0->getSize() > ht1->getSize()) {
            HashTable* tmp = ht0;
            ht0 = ht1;
            ht1 = tmp;
        }

        // scan ht0
        uint32_t index = ht0->getIndex(cursor);
        ht0->scanEntries(index, proc, priv);

        /* scan ht1: 由于ht1 > ht0, 所以ht1的大小是ht0的二倍
         * 1.以ht0为基准去进行迭代遍历，函数最后进位的时候也是以ht0的坐标去递进(使用了ht0的掩码),
         *   对于ht1中的遍历，只是查找ht1中与ht0中相对应的entry, 以防止遗漏
         * 2.ht1中有两个连续的entry与ht0中的entry相对应, 因此需要scan ht1中两个entry,
         *   例如ht0->size = 2, ht1->size = 4。那么ht0->mask = 0001, ht1->mask = 0011,
         *   此时ht0中与index=0001相对应的ht1中的元素是0001和0011
         * 3.ht1中的第二个scan就是将index的第二位置1, 以遍历相应的第二个桶
         */
        index = ht1->getIndex(cursor);
        ht1->scanEntries(index, proc, priv);
        index |= (~ht0->getMask() & ht1->getMask()) | (ht0->getMask() & ~ht1->getMask());
        ht1->scanEntries(index, proc, priv);
    } else {
        // 如果未处于rehash, 只访问ht[0]就可以了
        uint32_t index = ht0->getIndex(cursor);
        ht0->scanEntries(index, proc, priv);
    }

    cursor |= ~ht0->getMask();
    // 高位加1，向低位进位
    cursor = revBits(cursor);
    cursor++;
    cursor = revBits(cursor);
    return cursor;
}

// reserve bit位， 例如：b1 b2 b3 b4，经过reserve后变成b4 b3 b2 b1
uint32_t Dict::revBits(uint32_t bits) {
    uint32_t s = 8 * sizeof(bits); // bit size; must be power of 2
    uint32_t mask = ~0;
    while ((s >>= 1) > 0) {
        mask ^= (mask << s);
        bits = ((bits >> s) & mask) | ((bits << s) & ~mask);
    }
    return bits;
}

int Dict::expand(uint32_t size) {
    // todo dict size目前最大是32位
    uint64_t expandSize = nextPower(size);
    //if (expandSize > )
}

uint64_t Dict::nextPower(uint32_t num) {
    int i = HASH_TABLE_INITIAL_SIZE;
    while (1) {
        if (i >= num) {
            break;
        } else {
            i *= 2;
        }
    }

    return i;
}


unsigned int dictGenHashFunction(const char *buf, int len) {
    unsigned int hash = 5381;

    while (len--)
        hash = ((hash << 5) + hash) + (*buf++); /* hash * 33 + c */
    return hash;
}

uint64_t dictStrHash(const void *key) {
    if (NULL == key) {
        return 0;
    }
    return dictGenHashFunction(reinterpret_cast<const std::string*>(key)->c_str(),
                               reinterpret_cast<const std::string*>(key)->length());
}

int dictStrKeyCompare(const void *key1, const void *key2) {
    if (NULL == key1 && NULL == key2) {
        return 0;
    }
    if (NULL == key1) {
        return -1;
    }
    if (NULL == key2) {
        return 1;
    }

    const std::string *str1 = reinterpret_cast<const std::string*>(key1);
    return str1->compare(*reinterpret_cast<const std::string*>(key2));
}

void dictStrDestructor(void *val) {
    if (NULL == val) {
        return;
    }

    delete reinterpret_cast<std::string*>(val);
}

