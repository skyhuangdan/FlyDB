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
#include "../flyClient/FlyClientFactory.h"
#include "../flyObj/flyObjHashTable/FlyObjHashTableFactory.h"
#include "../flyObj/FlyObjLinkedList/FlyObjLinkedListFactory.h"
#include "../flyObj/FlyObjSkipList/FlyObjSkipListFactory.h"
#include "../flyObj/FlyObjIntSet/FlyObjIntSetFactory.h"
#include "../flyObj/FlyObjString/FlyObjStringFactory.h"
#include "../aof/AOFHandler.h"
#include "../flyServer/FlyServer.h"

Coordinator::Coordinator() {
    /** 加载config **/
    std::string configfile = "fly.conf";                    /** 配置文件名字 */
    AbstractConfigReader *configReader = new TextConfigReader(configfile);
    this->configCache = configReader->loadConfig();

    /** client factory **/
    this->flyClientFactory = new FlyClientFactory();

    /** fly obj factory **/
    this->flyObjHashTableFactory = new FlyObjHashTableFactory();
    this->flyObjLinkedListFactory = new FlyObjLinkedListFactory();
    this->flyObjSkipListFactory = new FlyObjSkipListFactory();
    this->flyObjIntSetFactory = new FlyObjIntSetFactory();
    this->flyObjStringFactory = new FlyObjStringFactory();

    /** net handler **/
    this->netHandler = NetHandler::getInstance();

    /** logger初始化 */
    FileLogFactory::init(configCache->getLogfile(),
                         configCache->getSyslogEnabled(),
                         configCache->getVerbosity());
    if (configCache->getSyslogEnabled()) {          /** syslog */
        openlog(configCache->getSyslogIdent(),
                LOG_PID | LOG_NDELAY | LOG_NOWAIT,
                configCache->getSyslogFacility());
    }

    /** fdb handler **/
    this->fdbHandler = new FDBHandler(this,
                                      configCache->getFdbFile(),
                                      CONFIG_LOADING_INTERVAL_BYTES);

    /** aof handler **/
    this->aofHandler = AOFHandler::Builder()
            .coordinator(this)
            .fileName(configCache->getAofFile())
            .state(configCache->getAofState())
            .useFdbPreamble(configCache->isAofUseFdbPreamble())
            .fsyncStragy(configCache->getAofFsync())
            .build();

    /** event loop **/
    this->flyServer = new FlyServer(this);
    this->eventLoop =
            new EventLoop(this, flyServer->getMaxClients() + CONFIG_FDSET_INCR);

    /** flyserver初始化 **/
    this->flyServer->init(configCache);

    /** fdb pipe init */
    this->fdbPipe = new Pipe(this);

    /** aof pipe init  */
    this->aofDataPipe = new Pipe(this);
    this->aofAckToParentPipe = new Pipe(this);
    this->aofAckToChildPipe = new Pipe(this);

    /** log handler */
    this->logHandler = logFactory->getLogger();
}

Coordinator::~Coordinator() {
    delete this->flyServer;
    delete this->eventLoop;
    delete this->fdbHandler;
    delete this->netHandler;
    delete this->aofHandler;
}

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

AbstractFlyClientFactory *Coordinator::getFlyClientFactory() const {
    return this->flyClientFactory;
}

AbstractFlyObjFactory *Coordinator::getFlyObjHashTableFactory() const {
    return flyObjHashTableFactory;
}

AbstractFlyObjFactory *Coordinator::getFlyObjLinkedListFactory() const {
    return flyObjLinkedListFactory;
}

AbstractFlyObjFactory *Coordinator::getFlyObjSkipListFactory() const {
    return flyObjSkipListFactory;
}

AbstractFlyObjFactory *Coordinator::getFlyObjIntSetFactory() const {
    return flyObjIntSetFactory;
}

AbstractFlyObjFactory *Coordinator::getFlyObjStringFactory() const {
    return flyObjStringFactory;
}

AbstractPipe *Coordinator::getChildInfoPipe() const {
    return this->fdbPipe;
}

AbstractLogHandler *Coordinator::getLogHandler() const {
    return this->logHandler;
}

AbstractBIOHandler *Coordinator::getBioHandler() const {
    return this->bioHandler;
}

AbstractPipe *Coordinator::getAofDataPipe() const {
    return this->aofDataPipe;
}

AbstractPipe *Coordinator::getAofAckToParentPipe() const {
    return this->aofAckToParentPipe;
}

AbstractPipe *Coordinator::getAofAckToChildPipe() const {
    return this->aofAckToChildPipe;
}
