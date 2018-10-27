//
// Created by 赵立伟 on 2018/10/20.
//

#ifndef FLYDB_TIMEEVENT_H
#define FLYDB_TIMEEVENT_H

#include "EventDef.h"

class TimeEvent {
public:
    TimeEvent(uint64_t id, long long milliseconds, timeEventProc *proc, void *clientData, eventFinalizerProc *finalizerProc);
    uint64_t getId() const;
    long getWhenSec() const;
    long getWhenMs() const;
    void *getClientData() const;
    bool operator< (const TimeEvent& timeEvent);
    bool operator== (const TimeEvent& timeEvent);
    bool operator== (const uint64_t id);
private:
    uint64_t id;
    long whenSec;
    long whenMs;
    timeEventProc *timeProc;
    eventFinalizerProc *finalizerProc;
    void *clientData;
};


#endif //FLYDB_TIMEEVENT_H
