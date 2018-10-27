//
// Created by 赵立伟 on 2018/10/20.
//

#include <cstdio>
#include "FileEvent.h"
#include "EventDef.h"
#include "Select.h"

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

int FileEvent::addFileProc(int fd, int mask, fileEventProc *proc, void *clientData) {
    if (NULL == clientData) {
        return -1;
    }

    // 设置相应的proc
    this->mask |= mask;
    if (mask & ES_READABLE) {
        this->rfileProc = proc;
    }
    if (mask & ES_WRITABLE) {
        this->wfileProc = proc;
    }
    this->clientData = clientData;

    // 设置监听fd
    EventState* eventState = (EventState*) clientData;
    eventState->add(fd, mask);

    return 1;
}

void FileEvent::delFileProc(int fd, int mask) {
    this->mask &= ~mask;
    // 设置监听fd
    EventState* eventState = (EventState*) clientData;
    eventState->del(fd, mask);
}

bool FileEvent::noneMask() {
    return ES_NONE == this->mask;
}
