//
// Created by 赵立伟 on 2018/10/20.
//

#include "FileEvent.h"

void FileEvent::setMask(int mask) {
    this->mask = mask;
}

void FileEvent::setClientData(void *clientData) {
    this->clientData = clientData;
}

void FileEvent::setRFileProc(fileProc *rfileProc) {
    this->rfileProc = rfileProc;
}

void FileEvent::setWFileProc(fileProc *wfileProc) {
    this->wfileProc = wfileProc;
}
