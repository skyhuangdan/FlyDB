//
// Created by 赵立伟 on 2018/12/8.
//

#include "Coordinator.h"
#include "../net/NetHandler.h"
#include "../config/TextConfigReader.h"
#include "../config/ConfigCache.h"
#include "../fdb/FDBHandler.h"
#include "../atomic/AtomicHandler.h"
#include "../flyClient/FlyClient.h"

AbstractNetHandler *Coordinator::getNetHandler() const {
    return this->netHandler;
}

AbstractFlyServer *Coordinator::getFlyServer() const {
    return this->flyServer;
}

AbstractEventLoop *Coordinator::getEventLoop() const {
    return this->eventLoop;
}

AbstractAOFHandler *Coordinator::getAofHandler() const {
    return this->aofHandler;
}

AbstractFDBHandler *Coordinator::getFdbHandler() const {
    return this->fdbHandler;
}

AbstractCoordinator* Coordinator::setNetHandler(AbstractNetHandler *netHandler) {
    this->netHandler = netHandler;
    return this;
}

AbstractCoordinator* Coordinator::setFlyServer(AbstractFlyServer *flyServer) {
    this->flyServer = flyServer;
    return this;
}

AbstractCoordinator* Coordinator::setEventLoop(AbstractEventLoop *eventLoop) {
    this->eventLoop = eventLoop;
    return this;
}

AbstractCoordinator* Coordinator::setAofHandler(AbstractAOFHandler *aofHandler) {
    this->aofHandler = aofHandler;
    return this;
}

AbstractCoordinator* Coordinator::setFdbHandler(AbstractFDBHandler *fdbHandler) {
    this->fdbHandler = fdbHandler;
    return this;
}

