//
// Created by 赵立伟 on 2018/10/5.
//

#include <iostream>
#include "ZipList.h"

ZipList::ZipList() {
    this->bytes = 0;
    this->entries = (unsigned char*)malloc(1);
    *(this->entries) = ZIP_END;
    this->len = 0;
    this->zltail = 1;
}

int ZipList::push(unsigned char* s, uint32_t len, int where) {

}

unsigned char* ZipList::getEntry(uint32_t index) {
    unsigned char* entry = this->entries;
    for (int i = 0; i < index; i++) {
        entry = nextEntry(entry);
        if (NULL == entry) {
            return NULL;
        }
    }

    return entry;
}

unsigned char* ZipList::findEntry(unsigned char* entry, uint32_t len, uint16_t skip) {

}

unsigned char* ZipList::nextEntry(unsigned char* entry) {
    // s为空或者s指向ZIP_END
    if (NULL == entry || ZIP_END == *this->entries) {
        return NULL;
    }

    return entry + decodeEntryLen(entry, decodePrevLenSize(entry));
}

unsigned char* ZipList::prevEntry(unsigned char* entry) {
    // entry为空或者指向头结点
    if (NULL == entry || entry == this->entries) {
        return NULL;
    }

    // entry指向尾节点
    if (ZIP_END == *entry) {
        return this->entries + this->zltail;
    }

    return entry - decodePrevEntryLength(entry);
}

int ZipList::merge(ZipList zl) {

}

uint16_t ZipList::length() {
    return this->len;
}

uint32_t ZipList::blobLen() {
    return this->bytes;
}

int ZipList::deleteEntry(unsigned char* entry) {
    // entry为空或者entry指向ZIP_END
    if (NULL == entry || ZIP_END == *entry) {
        return -1;
    }

    unsigned char* next = this->nextEntry(entry);
    if (ZIP_END == *next) {
        memmove(entry, next, 1);
        this->zltail = this->prevEntry(entry) - this->entries;
    } else {
        unsigned char* entryEncoding = getEntryEncoding(entry);
        unsigned char* nextEncoding = getEntryEncoding(next);
        memmove(entryEncoding, nextEncoding, this->entries + this->zltail - nextEncoding + 1);
        this->zltail -= nextEncoding - entryEncoding;
    }

    return 1;
}

uint32_t ZipList::decodePrevEntryLength(unsigned char* entry) {
    uint8_t size = decodePrevLenSize(entry);
    if (ZIP_PREVENTRY_SHORT == size) {
        return *entry;
    } else {
        return *(reinterpret_cast<uint32_t*>(entry + 1));
    }
}

unsigned char* ZipList::getEntryEncoding(unsigned char* entry) {
    return entry + this->decodePrevLenSize(entry);
}

uint8_t ZipList::decodePrevLenSize(unsigned char* entry) {
    if (NULL == entry) {
        return -1;
    }

    return ZIP_PREVENTRY_LONG_FLAG == *entry ? ZIP_PREVENTRY_LONG : ZIP_PREVENTRY_SHORT;
}

uint32_t ZipList::decodeEntryLen(unsigned char* entry, uint8_t prevLenSize) {
    unsigned char* encoding = entry + prevLenSize;
    uint8_t flag = *encoding & 0xc0;

    uint8_t encodingSize = 0;
    uint32_t contentSize = 0;
    switch (flag) {
        case 0x00:
            encodingSize = 1;
            contentSize = *encoding & 0x3F;
            break;
        case 0x40:
            encodingSize = 2;
            contentSize = *reinterpret_cast<uint16_t*>(encoding) & 0x3FFF;
            break;
        case 0x80:
            encodingSize = 5;
            contentSize = *reinterpret_cast<uint32_t*>(encoding + 1) & 0xFFFFFFFF;
            break;
        case 0xc0:
            encodingSize = 1;
            contentSize = 1;
            break;
    }

    return prevLenSize + encodingSize + contentSize;
}

