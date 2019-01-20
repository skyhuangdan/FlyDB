//
// Created by 赵立伟 on 2018/11/29.
//

#include "AOFHandler.h"

AOFHandler::AOFHandler(AbstractCoordinator *coordinator,
                       char *fileName,
                       AOFState state) {
    this->coordinator = coordinator;
    this->fileName = fileName;
    this->state = state;
}

pid_t AOFHandler::getChildPid() const {
    return this->childPid;
}

void AOFHandler::setChildPid(pid_t childPid) {
    this->childPid = childPid;
}

bool AOFHandler::haveChildPid() const {
    return -1 != this->childPid;
}

bool AOFHandler::IsStateOn() const {
    return this->state == AOF_ON;
}

void AOFHandler::setState(AOFState aofState) {
    this->state = aofState;
}

