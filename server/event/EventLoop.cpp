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

int EventLoop::createfileevent(int fd, int mask, fileProc* proc, void *clientdata) {
    if (fd > this->setSize) {
        return ER_ERR;
    }

    // todo:
}

int EventLoop::resizeSetSize(int setSize) {
    if (setSize == this->setSize) {
       return ER_OK;
    }

    // 如果当前最大fd > setSize, 则返回失败
    if (this->maxfd > setSize) {
        return ER_ERR;
    }

    this->fileEvents.resize(setSize);
    for (int i = 0; i < this->setSize; i++) {
        this->fileEvents[0].setMask(ES_NONE);
    }

    this->setSize = setSize;
    return ER_OK;
}
