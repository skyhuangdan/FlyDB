//
// Created by 赵立伟 on 2018/10/20.
//

#ifndef FLYDB_FILEEVENT_H
#define FLYDB_FILEEVENT_H


#include "EventDef.h"

class FileEvent {
public:
    void setMask(int mask);
    int getMask() const;
    void setClientData(void *clientData);
    void* getClientData() const;
    void setRFileProc(fileEventProc *rfileProc);
    fileEventProc* getRFileProc() const;
    void setWFileProc(fileEventProc *wfileProc);
    fileEventProc* getWFileProc() const;
    int addFileProc(int fd, int mask, fileEventProc *proc, void *clientData);
    void delFileProc(int fd, int mask);
    bool noneMask();

private:
    int mask;       // EVENT_READABLE或者EVENT_WRITABLE
    void *clientData;
    fileEventProc *rfileProc, *wfileProc;
};


#endif //FLYDB_FILEEVENT_H
