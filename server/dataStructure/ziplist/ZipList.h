//
// Created by 赵立伟 on 2018/10/5.
//

#ifndef FLYDB_ZIPLIST_H
#define FLYDB_ZIPLIST_H

class ZipList {
public:
    ZipList();
    int push(unsigned char* s, uint32_t len, int where);
    unsigned char* getEntry(uint32_t index);
    unsigned char* findEntry(unsigned char* s, uint32_t len, uint16_t skip);
    unsigned char* nextEntry(unsigned char* s);
    unsigned char* prevEntry(unsigned char* s);
    int merge(ZipList zl);
    uint16_t length();
    uint32_t blobLen();
    int deleteEntry(unsigned char* s);

private:
    uint32_t bytes;
    uint32_t zltail;
    uint16_t len;
    unsigned char* entries;
};

#endif //FLYDB_ZIPLIST_H
