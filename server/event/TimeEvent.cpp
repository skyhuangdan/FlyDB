//
// Created by 赵立伟 on 2018/10/20.
//

#include "TimeEvent.h"

TimeEvent::TimeEvent(uint64_t id, long long milliseconds, timeEventProc *proc, void *clientData, eventFinalizerProc *finalizerProc) {
    this->id = id;
    this->timeProc = proc;
    this->clientData = clientData;
    this->finalizerProc = finalizerProc;
    this->whenSec = milliseconds / 1000;
    this->whenMs = milliseconds % 1000;
}

uint64_t TimeEvent::getId() const {
    return id;
}

long TimeEvent::getWhenSec() const {
    return whenSec;
}

long TimeEvent::getWhenMs() const {
    return whenMs;
}

void *TimeEvent::getClientData() const {
    return clientData;
}

bool TimeEvent::operator< (const TimeEvent& timeEvent) {
    return this->whenSec < timeEvent.whenSec
           || (this->whenSec == timeEvent.whenSec && this->whenMs < timeEvent.whenMs);
}

bool TimeEvent::operator== (const TimeEvent& timeEvent) {
    return this->whenSec == timeEvent.whenSec && this->whenMs == timeEvent.whenMs;
}

bool TimeEvent::operator== (const uint64_t id) {
    return this->id == id;
}
