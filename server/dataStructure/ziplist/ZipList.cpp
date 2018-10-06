//
// Created by 赵立伟 on 2018/10/5.
//

#include "ZipList.h"

ZipList::ZipList() {
    this->bytes = 0;
    this->entries = NULL;
    this->len = 0;
    this->zltail = this->entries;
}

int ZipList::push(unsigned char* s, uint32_t len, int where) {

}
