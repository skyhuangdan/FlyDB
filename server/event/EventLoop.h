//
// Created by 赵立伟 on 2018/10/20.
//

#ifndef FLYDB_EVENTLOOP_H
#define FLYDB_EVENTLOOP_H

#include <vector>
#include <ctime>
#include "FileEvent.h"
#include "TimeEvent.h"

class EventLoop {
public:
    EventLoop(int setSize);
    ~EventLoop();
    int getSetSize() const;
    int resizeSetSize(int setSize);
    void stop();
    int createFileEvent(int fd, int mask, fileProc* proc, void *clientdata);
    int deleteFileEvent(int fd, int mask);

private:
    int maxfd;          // 当前注册的最大fd(file descriptor)
    int setSize;        // 最大fd数量
    long long timeEventNextId;
    time_t lastTime;
    std::vector<FileEvent> fileEvents;
    std::vector<TimeEvent> timeEvents;
    bool stopFlag;
    void *apiData;
};


#endif //FLYDB_EVENTLOOP_H
