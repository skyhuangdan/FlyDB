//
// Created by 赵立伟 on 2018/12/8.
//

#ifndef FLYDB_COORDINATOR_H
#define FLYDB_COORDINATOR_H

#include "interface/AbstractCoordinator.h"

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

    /** ChildInfo Pipe: child-->parent */
    AbstractPipe *getChildInfoPipe() const;
    
    /** LogHandler */
    AbstractLogHandler *getLogHandler() const;

    /** bio */
    AbstractBIOHandler *getBioHandler() const;

    /** AOF Pipe */
    AbstractPipe *getAofDataPipe() const;
    AbstractPipe *getAofAckToParentPipe() const;
    AbstractPipe *getAofAckToChildPipe() const;

    /** 关闭上述所有管道：childInfo pipe和AOF pipe */
    void closeAllPipe();
    int openAllPipe();

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
     * ChildInfo Pipe: child -> parent info
     **/
    AbstractPipe *fdbPipe = NULL;

    /**
     * AOF Pipe
     **/
     AbstractPipe *aofDataPipe = NULL;          // parent-->child: aof diff info
     AbstractPipe *aofAckToParentPipe = NULL;   // child-->parent: ack info
     AbstractPipe *aofAckToChildPipe = NULL;    // parent-->child: ack info
    
    /**
     * logHandler
     */
     AbstractLogHandler *logHandler = NULL;

     /**
      * bio
      **/
      AbstractBIOHandler *bioHandler = NULL;

};


#endif //FLYDB_COORDINATOR_H
