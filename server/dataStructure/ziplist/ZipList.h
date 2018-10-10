//
// Created by 赵立伟 on 2018/10/5.
//

#ifndef FLYDB_ZIPLIST_H
#define FLYDB_ZIPLIST_H

const uint8_t ZIP_END = 255;
const uint8_t ZIP_PREVENTRY_LONG_FLAG = 0XFE;
const uint8_t ZIP_PREVENTRY_LONG = 5;
const uint8_t ZIP_PREVENTRY_SHORT = 1;

class ZipList {
public:
    ZipList();
    int push(unsigned char* s, uint32_t len, int where);
    unsigned char* getEntry(uint32_t index);
    unsigned char* findEntry(unsigned char* entry, uint32_t len, uint16_t skip);
    unsigned char* nextEntry(unsigned char* entry);
    unsigned char* prevEntry(unsigned char* entry);
    int merge(ZipList zl);
    uint16_t length();
    uint32_t blobLen();
    int deleteEntry(unsigned char* entry);

private:
    uint32_t decodePrevEntryLength(unsigned char* entry);
    uint8_t decodePrevLenSize(unsigned char* entry);
    uint32_t decodeEntryLen(unsigned char* entry, uint8_t prevLenSize);
    uint32_t bytes;
    uint32_t zltail;
    uint16_t len;
    unsigned char* entries;
};

#endif //FLYDB_ZIPLIST_H
