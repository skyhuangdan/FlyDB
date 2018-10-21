//
// Created by 赵立伟 on 2018/10/20.
//

#include <cstdlib>
#include "EventLoop.h"
#include "Select.h"
#include "EventDef.h"

EventLoop::EventLoop(int setSize) {
    this->setSize = setSize;
    this->lastTime = time(NULL);
    this->timeEventNextId = 0;
    this->stopFlag = false;
    this->maxfd = -1;
    this->apiData = new EventState();
    this->fileEvents.resize(this->setSize);
    for (int i = 0; i < this->setSize; i++) {
        this->fileEvents[0].setMask(ES_NONE);
    }
}

EventLoop::~EventLoop() {
    delete this->apiData;
}

int EventLoop::getSetSize() const {
    return setSize;
}

void EventLoop::stop() {
    this->stopFlag = true;
}

int EventLoop::createFileEvent(int fd, int mask, fileProc* proc, void *clientdata) {
    if (fd > this->setSize) {
        return -1;
    }

    // 设置fileEvent, 添加file proc
    FileEvent& fileEvent = this->fileEvents[fd];
    fileEvent.addFileProc(fd, mask, proc, clientdata);

    if (this->maxfd < fd) {
        this->maxfd = fd;
    }

    return 1;
}

int EventLoop::deleteFileEvent(int fd, int mask) {
    if (fd > this->setSize) {
        return -1;
    }

    FileEvent &fileEvent = this->fileEvents[fd];
    if (ES_NONE == fileEvent.getMask()) {
        return -1;
    }

    fileEvent.delFileProc(fd, mask);

    if (fd == this->maxfd && this->fileEvents[fd].noneMask()) {
        for (int i = this->maxfd - 1; i >= 0; i--) {
            if (!this->fileEvents[i].noneMask()) {
                this->maxfd = i;
                break;
            }
        }
    }

    return 1;
}

int EventLoop::resizeSetSize(int setSize) {
    if (setSize == this->setSize) {
       return 1;
    }

    // 如果当前最大fd > setSize, 则返回失败
    if (this->maxfd > setSize) {
        return -1;
    }

    this->fileEvents.resize(setSize);
    for (int i = 0; i < this->setSize; i++) {
        this->fileEvents[0].setMask(ES_NONE);
    }

    this->setSize = setSize;
    return 1;
}
