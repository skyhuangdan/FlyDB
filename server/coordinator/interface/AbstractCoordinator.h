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
#include "../../flyClient/interface/AbstractFlyClientFactory.h"
#include "../../flyObj/interface/AbstractFlyObjFactory.h"
#include "../../pipe/interface/AbstractPipe.h"
#include "../../bio/interface/AbstractBIOHandler.h"

class AbstractFlyServer;
class AbstractNetHandler;
class AbstractFlyClientFactory;

class AbstractCoordinator {
public:

    virtual AbstractNetHandler *getNetHandler() const = 0;

    virtual AbstractFlyServer *getFlyServer() const = 0;

    virtual AbstractEventLoop *getEventLoop() const = 0;

    virtual AbstractAOFHandler *getAofHandler() const = 0;

    virtual AbstractFDBHandler *getFdbHandler() const = 0;

    virtual AbstractFlyClientFactory *getFlyClientFactory() const = 0;

    /** fly object factory **/
    virtual AbstractFlyObjFactory *getFlyObjHashTableFactory() const = 0;
    virtual AbstractFlyObjFactory *getFlyObjLinkedListFactory() const = 0;
    virtual AbstractFlyObjFactory *getFlyObjSkipListFactory() const = 0;
    virtual AbstractFlyObjFactory *getFlyObjIntSetFactory() const = 0;
    virtual AbstractFlyObjFactory *getFlyObjStringFactory() const = 0;

    /** Pipe */
    virtual AbstractPipe *getFDBPipe() const = 0;

    /** LogHandler */
    virtual AbstractLogHandler *getLogHandler() const = 0;

    /** bio */
    AbstractBIOHandler *getBioHandler() const;

};

#endif //FLYDB_ABSTRACTCOORDINATOR_H
