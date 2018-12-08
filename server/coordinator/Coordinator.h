//
// Created by 赵立伟 on 2018/12/8.
//

#ifndef FLYDB_COORDINATOR_H
#define FLYDB_COORDINATOR_H

#include "interface/AbstractCoordinator.h"
#include "../net/interface/AbstractNetHandler.h"
#include "../flyServer/FlyServer.h"
#include "../fdb/interface/AbstractFDBHandler.h"
#include "../config/interface/AbstractConfigReader.h"

class Coordinator : public AbstractCoordinator {
public:
    Coordinator();
    ~Coordinator();
    AbstractNetHandler *getNetHandler() const;
    AbstractFlyServer *getFlyServer() const;
    AbstractEventLoop *getEventLoop() const;
    AbstractAOFHandler *getAofHandler() const;
    AbstractFDBHandler *getFdbHandler() const;
private:

    AbstractNetHandler *netHandler;
    AbstractFlyServer *flyServer;
    AbstractEventLoop *eventLoop;
    AbstractAOFHandler *aofHandler;
    AbstractFDBHandler *fdbHandler;

};


#endif //FLYDB_COORDINATOR_H
