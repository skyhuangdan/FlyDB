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

int FileEvent::addFileProc(int mask, fileEventProc *proc, void *clientData) {
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

    return 1;
}

void FileEvent::delFileProc(int mask) {
    this->mask &= ~mask;
    // 设置监听fd
    PollState* eventState = (PollState*) clientData;
    eventState->del(fd, mask);
}

bool FileEvent::noneMask() {
    return ES_NONE == this->mask;
}

void FileEvent::process(int mask) {
    int rfired = 0;

    // 如果是有可读/可写事件，则执行事件回调
    if (mask & ES_READABLE) {
        rfired = 1;
        rfileProc(this->eventLoop, this->fd, clientData, mask);
    }
    if (mask & ES_WRITABLE) {
        if (this->wfileProc != this->rfileProc || 0 == rfired) {
            wfileProc(this->eventLoop, fd, clientData, mask);
        }
    }
}

int FileEvent::getFd() const {
    return this->fd;
}

void FileEvent::setFd(int fd) {
    this->fd = fd;
}

EventLoop *FileEvent::getEventLoop() const {
    return eventLoop;
}

void FileEvent::setEventLoop(EventLoop *eventLoop) {
    FileEvent::eventLoop = eventLoop;
}
