//
// Created by 赵立伟 on 2018/10/20.
//

#ifndef FLYDB_FILEEVENT_H
#define FLYDB_FILEEVENT_H

#include "EventDef.h"
#include "interface/AbstractEventLoop.h"

struct FileEvent {
public:
    FileEvent();
    void setMask(int mask);
    int getMask() const;
    void setClientData(void *clientData);
    void* getClientData() const;
    void setRFileProc(fileEventProc *rfileProc);
    fileEventProc* getRFileProc() const;
    void setWFileProc(fileEventProc *wfileProc);
    fileEventProc* getWFileProc() const;
    int addFileProc(int mask, fileEventProc *proc, void *clientData);
    void delFileProc(int mask);
    bool noneMask();
    void process(int mask);
    int getFd() const;
    void setFd(int fd);
    void setCoordinator(const AbstractCoordinator *coordinator);

    int fd;
    int mask;       // EVENT_READABLE或者EVENT_WRITABLE
    void *clientData;
    fileEventProc *rfileProc, *wfileProc;
    const AbstractCoordinator *coordinator;
};


#endif //FLYDB_FILEEVENT_H