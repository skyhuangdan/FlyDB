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

Coordinator::Coordinator() {
    // 加载config
    std::string configfile = "fly.conf";                    /** 配置文件名字 */
    AbstractConfigReader *configReader = new TextConfigReader(configfile);
    ConfigCache *configCache = configReader->loadConfig();

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

    // fdb handler
    this->fdbHandler = new FDBHandler(this,
                                      configCache->getFdbFile(),
                                      CONFIG_LOADING_INTERVAL_BYTES);

    // event loop
    this->flyServer = new FlyServer(this);
    this->eventLoop =
            new EventLoop(this, flyServer->getMaxClients() + CONFIG_FDSET_INCR);

    // flyserver初始化
    this->flyServer->init(configCache);
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
