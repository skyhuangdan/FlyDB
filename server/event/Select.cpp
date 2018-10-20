//
// Created by 赵立伟 on 2018/10/20.
//

#include "Select.h"

EventState::EventState() {
    FD_ZERO(&(this->rfds));
    FD_ZERO(&(this->wfds));
}
