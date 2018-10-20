//
// Created by 赵立伟 on 2018/10/20.
//

#ifndef FLYDB_FILEEVENT_H
#define FLYDB_FILEEVENT_H

typedef void fileProc(struct EventLoop *eventLoop, int fd, void *clientdata, int mask);

class FileEvent {
public:
    void setMask(int mask);
    void setClientData(void *clientData);
    void setRFileProc(fileProc *rfileProc);
    void setWFileProc(fileProc *wfileProc);


private:
    int mask;       // EVENT_READABLE或者EVENT_WRITABLE
    void *clientData;
    fileProc *rfileProc, *wfileProc;
};


#endif //FLYDB_FILEEVENT_H
