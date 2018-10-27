//
// Created by 赵立伟 on 2018/10/20.
//

#ifndef FLYDB_SELECT_H
#define FLYDB_SELECT_H

#include <sys/select.h>
#include "EventLoop.h"

class PollState {
public:
    PollState();
    void add(int fd, int mask);
    void del(int fd, int mask);
    int poll(EventLoop *eventLoop, struct timeval *tvp);

private:
    fd_set rfds, wfds;
    // 当select()之后再去使用fd会不安全, 因此保留一个fd的备份
    fd_set _rfds, _wfds;
};

#endif //FLYDB_SELECT_H
