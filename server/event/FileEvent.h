//
// Created by 赵立伟 on 2018/10/20.
//

#ifndef FLYDB_FILEEVENT_H
#define FLYDB_FILEEVENT_H

typedef void fileProc(struct EventLoop *eventLoop, int fd, void *clientdata, int mask);

class FileEvent {
public:
    void setMask(int mask);
    int getMask() const;
    void setClientData(void *clientData);
    void* getClientData() const;
    void setRFileProc(fileProc *rfileProc);
    fileProc* getRFileProc() const;
    void setWFileProc(fileProc *wfileProc);
    fileProc* getWFileProc() const;
    int addFileProc(int fd, int mask, fileProc *proc, void *clientData);
    void delFileProc(int fd, int mask);
    bool noneMask();

private:
    int mask;       // EVENT_READABLE或者EVENT_WRITABLE
    void *clientData;
    fileProc *rfileProc, *wfileProc;
};


#endif //FLYDB_FILEEVENT_H
