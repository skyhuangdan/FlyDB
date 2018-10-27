//
// Created by 赵立伟 on 2018/10/20.
//

#include <cstring>
#include "Select.h"
#include "EventDef.h"

PollState::PollState() {
    FD_ZERO(&(this->rfds));
    FD_ZERO(&(this->wfds));
}

void PollState::add(int fd, int mask) {
    if (mask & ES_READABLE) {
        FD_SET(fd, &this->rfds);
    }
    if (mask & ES_WRITABLE) {
        FD_SET(fd, &this->wfds);
    }
}

void PollState::del(int fd, int mask) {
    if (mask & ES_READABLE) {
        FD_CLR(fd, &this->rfds);
    }
    if (mask & ES_WRITABLE) {
        FD_CLR(fd, &this->wfds);
    }
}

int PollState::poll(EventLoop *eventLoop, struct timeval *tvp) {
    // 保存副本
    memcpy(&this->_rfds, &this->rfds, sizeof(fd_set));
    memcpy(&this->_wfds, &this->wfds, sizeof(fd_set));

    // select并遍历获取事件
    int numEvents = 0;
    int retval = select(eventLoop->getMaxfd() + 1, &this->_rfds, &this->_wfds, NULL, tvp);
    if (retval > 0) {
        for (int i = 0; i <= eventLoop->getMaxfd(); i++) {
            int mask = 0;
            int eventMask = eventLoop->getFileEvents(i);

            if (ES_NONE == eventMask) {
                continue;
            }
            if (eventMask & ES_READABLE && FD_ISSET(i, &this->_rfds)) {
                mask |= ES_READABLE;
            }
            if (eventMask & ES_WRITABLE && FD_ISSET(i, &this->_wfds)) {
                mask |= ES_WRITABLE;
            }

            eventLoop->addFiredEvent(i, mask);
            numEvents++;
        }
    }

    return numEvents;
}