//
// Created by 赵立伟 on 2018/10/20.
//

#ifndef FLYDB_EVENTLOOP_H
#define FLYDB_EVENTLOOP_H

#include <vector>
#include <list>
#include <ctime>
#include "FileEvent.h"
#include "TimeEvent.h"
#include "FiredEvent.h"

class EventLoop;
typedef void beforeAndAfterSleepProc(EventLoop *eventLoop);

class EventLoop {
public:
    EventLoop(int setSize);
    ~EventLoop();
    int processEvents(int flags);
    void eventMain();
    int getMaxfd() const;

    // file event
    int getSetSize() const;
    int resizeSetSize(int setSize);
    void stop();
    int createFileEvent(int fd, int mask, fileEventProc* proc, void *clientdata);
    int deleteFileEvent(int fd, int mask);
    int getFileEvents(int fd);
    beforeAndAfterSleepProc* getBeforeSleepProc() const;
    void setBeforeSleepProc(beforeAndAfterSleepProc* proc);
    beforeAndAfterSleepProc* getAfterSleepProc() const;
    void setAfterSleepProc(beforeAndAfterSleepProc* proc);

    // time event
    int processTimeEvents();
    int deleteTimeEvent(uint64_t id);
    void createTimeEvent(long long milliseconds, timeEventProc *proc,
                        void *clientData, eventFinalizerProc *finalizerProc);

    // fired event
    void addFiredEvent(int fd, int mask);

private:
    int maxfd;          // 当前注册的最大fd(file descriptor)
    int setSize;        // 最大fd数量
    uint64_t timeEventNextId;
    int64_t lastTime;
    std::vector<FileEvent> fileEvents;
    std::vector<FiredEvent> firedEvent;
    std::list<TimeEvent> timeEvents;
    bool stopFlag;
    void *apiData;
    beforeAndAfterSleepProc *beforeSleepProc;
    beforeAndAfterSleepProc *afterSleepProc;
};


#endif //FLYDB_EVENTLOOP_H
