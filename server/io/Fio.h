//
// Created by 赵立伟 on 2018/12/1.
//

#ifndef FLYDB_FIO_H
#define FLYDB_FIO_H

#include <cstddef>
#include <cstdint>

class Fio {
public:
    virtual size_t read(struct _rio *, void *buf, size_t len) = 0;
    virtual size_t write(struct _rio *, const void *buf, size_t len) = 0;
    virtual int tell(struct _rio *) = 0;
    virtual int flush(struct _rio *) = 0;
    virtual void updateChecksum(struct _rio *, const void *buf, size_t len) {
        return;
    };

private:
    uint64_t checksum;
    size_t processedBytes;
    size_t maxProcessingChunk;
};

#endif //FLYDB_FIO_H
