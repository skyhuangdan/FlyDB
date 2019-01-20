//
// Created by 赵立伟 on 2018/11/29.
//

#include "AOFHandler.h"

AOFHandler::AOFHandler(AbstractCoordinator *coordinator,
                       char *fileName,
                       AOFState aofState) {
    this->coordinator = coordinator;
    this->fileName = fileName;
    this->aofState = aofState;
}

pid_t AOFHandler::getAofChildPid() const {
    return this->aofChildPid;
}

void AOFHandler::setAofChildPid(pid_t aofChildPid) {
    this->aofChildPid = aofChildPid;
}

bool AOFHandler::haveAofChildPid() const {
    return -1 != this->aofChildPid;
}

bool AOFHandler::IsAofStateOn() const {
    return this->aofState == AOF_ON;
}

void AOFHandler::setAofState(AOFState aofState) {
    this->aofState = aofState;
}

