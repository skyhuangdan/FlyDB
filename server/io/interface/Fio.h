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
    Fio();

    virtual int tell() = 0;                         // get offset
    virtual int flush() = 0;                        // flush data to device
    size_t write(const void *buf, size_t len);
    size_t read(void *buf, size_t len);
    virtual int updateChecksum(const void *buf, size_t len);

    void setMaxProcessingChunk(uint64_t maxProcessingChunk);
    uint64_t getMaxProcessingChunk() const;
    uint64_t getChecksum() const;
    void setChecksum(uint64_t checksum);
    size_t getProcessedBytes() const;
    bool haveProcessedBytes() const;
    void setProcessedBytes(size_t processedBytes);
    void addProcessedBytes(size_t addBytes);

protected:
    virtual size_t baseread(void *buf, size_t len) = 0;
    virtual size_t basewrite(const void *buf, size_t len) = 0;

private:
    uint64_t checksum = 0;
    size_t processedBytes = 0;
    uint64_t maxProcessingChunk = 0;
};

#endif //FLYDB_FIO_H
