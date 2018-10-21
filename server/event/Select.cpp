//
// Created by 赵立伟 on 2018/10/20.
//

#include "Select.h"
#include "EventDef.h"

EventState::EventState() {
    FD_ZERO(&(this->rfds));
    FD_ZERO(&(this->wfds));
}

void EventState::add(int fd, int mask) {
    if (mask & ES_READABLE) {
        FD_SET(fd, &this->rfds);
    }
    if (mask & ES_WRITABLE) {
        FD_SET(fd, &this->wfds);
    }
}

void EventState::del(int fd, int mask) {
    if (mask & ES_READABLE) {
        FD_CLR(fd, &this->rfds);
    }
    if (mask & ES_WRITABLE) {
        FD_CLR(fd, &this->wfds);
    }
}
