//
// Created by 赵立伟 on 2018/12/1.
//

#include "Fio.h"

Fio::Fio(uint64_t maxProcessingChunk) {
    this->maxProcessingChunk = maxProcessingChunk;
}

int Fio::updateChecksum(const void *buf, size_t len) {
    return -1;
}
