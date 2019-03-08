//
// Created by 赵立伟 on 2018/10/20.
//

#include <cstdio>
#include "FileEvent.h"
#include "EventDef.h"
#include "Select.h"

FileEvent::FileEvent() {
}

void FileEvent::setMask(int mask) {
    this->mask = mask;
}

int FileEvent::getMask() const {
    return this->mask;
}

void FileEvent::setClientData(void *clientData) {
    this->clientData = clientData;
}

void* FileEvent::getClientData() const {
    return this->clientData;
}

fileEventProc* FileEvent::getRFileProc() const {
    return this->rfileProc;
}

fileEventProc* FileEvent::getWFileProc() const {
    return this->wfileProc;
}

void FileEvent::setRFileProc(fileEventProc *rfileProc) {
    this->rfileProc = rfileProc;
}

void FileEvent::setWFileProc(fileEventProc *wfileProc) {
    this->wfileProc = wfileProc;
}

int FileEvent::addFileProc(int mask, fileEventProc *proc, void *clientData) {
    // 设置相应的proc
    this->mask |= mask;
    if (mask & ES_READABLE) {
        this->rfileProc = proc;
    }
    if (mask & ES_WRITABLE) {
        this->wfileProc = proc;
    }
    this->clientData = clientData;

    return 1;
}

void FileEvent::delFileProc(int mask) {
    this->mask &= ~mask;
}

bool FileEvent::noneMask() {
    return ES_NONE == this->mask;
}

void FileEvent::process(int mask) {
    int rfired = 0;

    // 如果是有可读/可写事件，则执行事件回调
    if (mask & ES_READABLE) {
        rfired = 1;
        rfileProc(this->coordinator, this->fd, clientData, mask);
    }
    if (mask & ES_WRITABLE) {
        if (this->wfileProc != this->rfileProc || 0 == rfired) {
            wfileProc(this->coordinator, fd, clientData, mask);
        }
    }
}

int FileEvent::getFd() const {
    return this->fd;
}

void FileEvent::setFd(int fd) {
    this->fd = fd;
}

void FileEvent::setCoordinator(const AbstractCoordinator *coordinator) {
    this->coordinator = coordinator;
}
