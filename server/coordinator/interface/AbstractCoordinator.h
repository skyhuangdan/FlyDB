//
// Created by 赵立伟 on 2018/12/8.
//

#ifndef FLYDB_ABSTRACTCOORDINATOR_H
#define FLYDB_ABSTRACTCOORDINATOR_H

#include "../../event/interface/AbstractEventLoop.h"
#include "../../flyServer/interface/AbstractFlyServer.h"
#include "../../net/interface/AbstractNetHandler.h"
#include "../../aof/interface/AbstractAOFHandler.h"
#include "../../fdb/interface/AbstractFDBHandler.h"
#include "../../config/interface/AbstractConfigReader.h"

class AbstractFlyServer;
class AbstractNetHandler;

class AbstractCoordinator {
public:

    virtual AbstractNetHandler *getNetHandler() const = 0;

    virtual AbstractFlyServer *getFlyServer() const = 0;

    virtual AbstractEventLoop *getEventLoop() const = 0;

    virtual AbstractAOFHandler *getAofHandler() const = 0;

    virtual AbstractFDBHandler *getFdbHandler() const = 0;
};

#endif //FLYDB_ABSTRACTCOORDINATOR_H
