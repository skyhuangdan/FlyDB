//
// Created by 赵立伟 on 2018/10/20.
//

#ifndef FLYDB_EVENTDEF_H
#define FLYDB_EVENTDEF_H
#include <cstdint>
#include <sys/time.h>

enum EventStatus {
    ES_NONE = 0,
    ES_READABLE,
    ES_WRITABLE
};

const int EVENT_FILE_EVENTS = 1;
const int EVENT_TIME_EVENTS = 2;
const int EVENT_ALL_EVENTS = EVENT_FILE_EVENTS | EVENT_TIME_EVENTS;
const int EVENT_CALL_AFTER_SLEEP = 8;
const int EVENT_DONT_WAIT = 4;

class EventLoop;
typedef int timeEventProc(EventLoop *eventLoop, uint64_t id, void *clientData);
typedef void fileEventProc(EventLoop *eventLoop, int fd, void *clientdata, int mask);
typedef void eventFinalizerProc(EventLoop *eventLoop, void *clientData);

#endif //FLYDB_EVENTDEF_H
