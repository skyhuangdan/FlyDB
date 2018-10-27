//
// Created by 赵立伟 on 2018/10/27.
//

#include "FiredEvent.h"

FiredEvent::FiredEvent(int fd, int mask) {
    this->fd = fd;
    this->mask = mask;
}

int FiredEvent::getFd() const {
    return fd;
}

void FiredEvent::setFd(int fd) {
    FiredEvent::fd = fd;
}

int FiredEvent::getMask() const {
    return mask;
}

void FiredEvent::setMask(int mask) {
    FiredEvent::mask = mask;
}
