//
// Created by 赵立伟 on 2018/10/20.
//

#ifndef FLYDB_SELECT_H
#define FLYDB_SELECT_H

#include <sys/select.h>

class EventState {
public:
    EventState();

private:
    fd_set rfds, wfds;
    // 当select()之后再去使用fd会不安全, 因此保留一个fd的备份
    fd_set _rfds, _wfds;
};

class Select {

};


#endif //FLYDB_SELECT_H
