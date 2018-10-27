//
// Created by 赵立伟 on 2018/10/20.
//

#ifndef FLYDB_EVENTDEF_H
#define FLYDB_EVENTDEF_H
#include <cstdint>

enum EventStatus {
    ES_NONE = 0,
    ES_READABLE,
    ES_WRITABLE
};

const int EVENT_FILE_EVENTS = 1;
const int EVENT_TIME_EVENTS = 2;
const int EVENT_ALL_EVENTS = EVENT_FILE_EVENTS | EVENT_TIME_EVENTS;
const int EVENT_CALL_AFTER_SLEEP = 8;

typedef int timeEventProc(struct EventLoop *eventLoop, uint64_t id, void *clientData);
typedef void fileEventProc(struct EventLoop *eventLoop, int fd, void *clientdata, int mask);
typedef void eventFinalizerProc(struct EventLoop *eventLoop, void *clientData);

#endif //FLYDB_EVENTDEF_H
