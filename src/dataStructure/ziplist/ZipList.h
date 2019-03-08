//
// Created by 赵立伟 on 2018/10/5.
//

#ifndef FLYDB_ZIPLIST_H
#define FLYDB_ZIPLIST_H
#include <cstdint>

const uint8_t ZIP_END = 255;
const uint8_t ZIP_PREVENTRY_LONG_FLAG = 0XFE;
const uint8_t ZIP_PREVENTRY_LONG = 5;
const uint8_t ZIP_PREVENTRY_SHORT = 1;

class ZipList {
public:
    ZipList();
    int push(unsigned char* s, uint32_t len);
    unsigned char* getEntry(uint32_t index);
    unsigned char* nextEntry(unsigned char* entry);
    unsigned char* prevEntry(unsigned char* entry);
    uint16_t length();
    uint32_t blobLen();
    int deleteEntry(unsigned char* entry);

private:
    uint32_t decodePrevEntryLength(unsigned char* entry);
    unsigned char* getEntryEncoding(unsigned char* entry);
    uint8_t decodePrevLenSize(unsigned char* entry);
    uint32_t decodeEntryLen(unsigned char* entry, uint8_t prevLenSize);
    uint8_t decodeEncodingSize(unsigned char* entry, uint8_t prevLenSize);
    uint32_t zltail;
    uint16_t len;
    unsigned char* entries;
};

#endif //FLYDB_ZIPLIST_H
