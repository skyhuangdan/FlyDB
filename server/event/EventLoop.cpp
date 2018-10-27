//
// Created by 赵立伟 on 2018/10/20.
//

#include <cstdlib>
#include "EventLoop.h"
#include "Select.h"
#include "EventDef.h"

EventLoop::EventLoop(int setSize) {
    this->setSize = setSize;
    this->lastTime = getCurrentTime();
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
    this->timeEvents.clear();
    this->fileEvents.clear();
}

int EventLoop::processEvents(int flags) {
}

void EventLoop::eventMain() {
    this->stopFlag = 0;
    while (!this->stopFlag) {
        if (NULL != this->beforeSleepProc) {
            this->beforeSleepProc(this);
        }
        this->processEvents(EVENT_ALL_EVENTS | EVENT_CALL_AFTER_SLEEP);
    }
}

int EventLoop::getSetSize() const {
    return setSize;
}

void EventLoop::stop() {
    this->stopFlag = true;
}

int EventLoop::createFileEvent(int fd, int mask, fileEventProc* proc, void *clientdata) {
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

int EventLoop::getFileEvents(int fd) {
    if (fd > this->setSize) {
        return -1;
    }

    return this->fileEvents[fd].getMask();
}

beforeAndAfterSleepProc* EventLoop::getBeforeSleepProc() const {
    return this->beforeSleepProc;
}

void EventLoop::setBeforeSleepProc(beforeAndAfterSleepProc* proc) {
    this->beforeSleepProc = beforeSleepProc;
}

beforeAndAfterSleepProc* EventLoop::getAfterSleepProc() const {
    return this->afterSleepProc;
}

void EventLoop::setAfterSleepProc(beforeAndAfterSleepProc* proc) {
    this->afterSleepProc = afterSleepProc;
}

int EventLoop::processTimeEvents() {
    int64_t nowt = getCurrentTime();

    /** 如果系统时间曾经往后调、然后又调回来过，那么如果不处理的话，所有的timeevent处理时间可能会非常延后
     * 所以这里将处理时间置0，即马上处理定时任务，防止处理太滞后。**/
    if (nowt < this->lastTime) {
        for (auto iter : this->timeEvents) {
            iter.setWhen(0);
        }
    }

    bool needSort = false;
    for (auto iter : this->timeEvents) {
        if (iter.getWhen() < nowt) {
            int ret = iter.getTimeProc()(this, iter.getId(), iter.getClientData());
            if (ret > 0) {
                iter.setWhen(nowt + ret);
                needSort = true;
            } else {
                this->deleteTimeEvent(iter.getId());
            }
        }
    }
    if (needSort) {
        this->timeEvents.sort();
    }
}

int EventLoop::deleteTimeEvent(uint64_t id) {
    std::list<TimeEvent>::iterator iter = this->timeEvents.begin();
    for (iter; iter != this->timeEvents.end(); iter++) {
        if (iter->getId() == id) {
            if (iter->getFinalizerProc()) {
                iter->getFinalizerProc()(this, iter->getClientData());
            }
            this->timeEvents.erase(iter++);
            return 1;
        }
    }

    return -1;
}

void EventLoop::createTimeEvent(long long milliseconds, timeEventProc *proc,
                    void *clientData, eventFinalizerProc *finalizerProc) {
    this->timeEvents.push_front(TimeEvent(this->timeEventNextId++, milliseconds, proc, clientData, finalizerProc));
    this->timeEvents.sort();
}

