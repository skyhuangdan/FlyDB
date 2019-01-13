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
#include "../flyObj/interface/AbstractFlyObjFactory.h"

class Coordinator : public AbstractCoordinator {
public:
    Coordinator();
    ~Coordinator();

    AbstractNetHandler *getNetHandler() const;
    AbstractFlyServer *getFlyServer() const;
    AbstractEventLoop *getEventLoop() const;
    AbstractAOFHandler *getAofHandler() const;
    AbstractFDBHandler *getFdbHandler() const;
    AbstractFlyClientFactory *getFlyClientFactory() const;

    /** fly object factory **/
    AbstractFlyObjFactory *getFlyObjHashTableFactory() const;
    AbstractFlyObjFactory *getFlyObjLinkedListFactory() const;
    AbstractFlyObjFactory *getFlyObjSkipListFactory() const;
    AbstractFlyObjFactory *getFlyObjIntSetFactory() const;
    AbstractFlyObjFactory *getFlyObjStringFactory() const;

    /** Pipe */
    AbstractPipe *getPipe() const;

private:
    AbstractNetHandler *netHandler;
    AbstractFlyServer *flyServer;
    AbstractEventLoop *eventLoop;
    AbstractAOFHandler *aofHandler;
    AbstractFDBHandler *fdbHandler;
    AbstractFlyClientFactory *flyClientFactory;
    AbstractFlyObjFactory *flyObjHashTableFactory;
    AbstractFlyObjFactory *flyObjLinkedListFactory;
    AbstractFlyObjFactory *flyObjSkipListFactory;
    AbstractFlyObjFactory *flyObjIntSetFactory;
    AbstractFlyObjFactory *flyObjStringFactory;
    ConfigCache *configCache;

    /**
     * Pipe: child -> parent info sharing
     **/
    AbstractPipe *pipe = NULL;
};


#endif //FLYDB_COORDINATOR_H
