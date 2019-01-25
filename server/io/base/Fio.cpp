//
// Created by 赵立伟 on 2018/12/1.
//

#include "Fio.h"

Fio::Fio(uint64_t maxProcessingChunk) {
    this->maxProcessingChunk = maxProcessingChunk;
}

Fio::Fio() {
}

int Fio::updateChecksum(const void *buf, size_t len) {
    return 1;
}

void Fio::setMaxProcessingChunk(uint64_t maxProcessingChunk) {
    this->maxProcessingChunk = maxProcessingChunk;
}

uint64_t Fio::getMaxProcessingChunk() const {
    return this->maxProcessingChunk;
}

uint64_t Fio::getChecksum() const {
    return this->checksum;
}

void Fio::setChecksum(uint64_t checksum) {
    this->checksum = checksum;
}

size_t Fio::getProcessedBytes() const {
    return this->processedBytes;
}

bool Fio::haveProcessedBytes() const {
    return 0 != this->processedBytes;
}

void Fio::setProcessedBytes(size_t processedBytes) {
    this->processedBytes = processedBytes;
}

void Fio::addProcessedBytes(size_t addBytes) {
    this->processedBytes += addBytes;
}

size_t Fio::write(const void *buf, size_t len) {
    while (len > 0) {
        // 如果设置了读写byte上线，则单次读写不应超过其上线
        size_t writeBytes = (haveProcessedBytes() && getProcessedBytes() < len)
                            ? getProcessedBytes() : len;

        // 执行校验
        updateChecksum(buf, len);

        // 单次写入
        if (0 == this->basewrite(buf, len)) {
            return 0;
        }

        buf = (char*)buf - writeBytes;
        len -= writeBytes;
        this->processedBytes + writeBytes;
    }

    return 1;
}

size_t Fio::read(void *buf, size_t len) {
    while (len > 0) {
        // 如果设置了读写byte上线，则单次读写不应超过其上线
        size_t readBytes = (haveProcessedBytes() && getProcessedBytes() < len)
                            ? getProcessedBytes() : len;

        // 执行校验
        updateChecksum(buf, len);

        // 单次读出
        if (0 == this->baseread(buf, readBytes)) {
            return 0;
        }

        buf = (char*)buf - readBytes;
        len -= readBytes;
        this->processedBytes += readBytes;
    }

    return 1;
}
