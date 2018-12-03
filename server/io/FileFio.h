//
// Created by 赵立伟 on 2018/12/1.
//

#ifndef FLYDB_FILEFIO_H
#define FLYDB_FILEFIO_H

#include <cstdio>
#include "interface/Fio.h"

class FileFio : public Fio {
public:
    FileFio(FILE *fp, uint64_t maxProcessingChunk);
    size_t read(void *buf, size_t len);
    size_t write(const void *buf, size_t len);
    int tell();
    int flush();
    int updateChecksum(const void *buf, size_t len);

private:
    FILE *fp = NULL;
    off_t buffered = 0;                     // 距离上一次fsync所写入的字节数
    off_t autosync = 0;                     // 当写入数据>autosync时，执行fsync
};


#endif //FLYDB_FILEFIO_H
