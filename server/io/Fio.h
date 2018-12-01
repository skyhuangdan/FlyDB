//
// Created by 赵立伟 on 2018/12/1.
//

#ifndef FLYDB_FIO_H
#define FLYDB_FIO_H

#include <cstddef>
#include <cstdint>

class Fio {
public:
    Fio(uint64_t maxProcessingChunk);
    virtual size_t read(void *buf, size_t len) = 0;
    virtual size_t write(const void *buf, size_t len) = 0;
    virtual int tell() = 0;                         // get offset
    virtual int flush() = 0;                        // flush data to device
    virtual int updateChecksum(const void *buf, size_t len);

private:
    uint64_t checksum = 0;
    size_t processedBytes = 0;
    uint64_t maxProcessingChunk = 0;
};

#endif //FLYDB_FIO_H
