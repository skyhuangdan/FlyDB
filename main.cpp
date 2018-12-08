#include <iostream>

#include "server/flyServer/FlyServer.h"
#include "server/dataStructure/dict/Dict.h"
#include "server/dataStructure/dict/test/TestDictType.h"
#include "server/dataStructure/skiplist/SkipList.h"
#include "server/dataStructure/skiplist/test/TestSkipListType.h"
#include "server/dataStructure/intset/IntSet.h"
#include "server/utils/MiscTool.h"
#include "def.h"
#include "server/config/TextConfigReader.h"
#include "server/net/NetHandler.h"
#include "server/fdb/FDBHandler.h"
#include "server/coordinator/Coordinator.h"

/**
 * flyDB，取名fly有两层含义：
 *  第一："飞"谐音"菲"
 *  第二："飞"寓意飞快，代表其高性能
 * created by zlw, 20180918
 */

AbstractLogFactory *logFactory = new FileLogFactory();

int main(int argc, char **argv) {
    std::cout << "Hello, flyDB. Wish you be better!" << std::endl;

    // 加载config
    std::string configfile = "fly.conf";                    /** 配置文件名字 */
    AbstractConfigReader *configReader = new TextConfigReader(configfile);
    ConfigCache *configCache = configReader->loadConfig();

    AbstractNetHandler *netHandler = NetHandler::getInstance();

    /** logger初始化 */
    FileLogFactory::init(configCache->getLogfile(),
                         configCache->getSyslogEnabled(),
                         configCache->getVerbosity());
    // syslog
    if (configCache->getSyslogEnabled()) {
        openlog(configCache->getSyslogIdent(),
                LOG_PID | LOG_NDELAY | LOG_NOWAIT,
                configCache->getSyslogFacility());
    }

    AbstractCoordinator *coordinator = new Coordinator();

    // flyserver初始化
    FlyServer *flyServer = new FlyServer(coordinator);

    // fdb handler
    AbstractFDBHandler *fdbHandler = new FDBHandler(coordinator,
                                                    configCache->getFdbFile(),
                                                    CONFIG_LOADING_INTERVAL_BYTES);

    // event loop
    AbstractEventLoop *eventLoop = new EventLoop(coordinator,
            flyServer->getMaxClients() + CONFIG_FDSET_INCR);

    // 设置协调者
    coordinator->setEventLoop(eventLoop)
            ->setFdbHandler(fdbHandler)
            ->setFlyServer(flyServer)
            ->setNetHandler(netHandler);

    // 进入时间处理循环
    flyServer->init(configCache);
    eventLoop->eventMain();


    delete flyServer;
    delete eventLoop;
    delete fdbHandler;
    delete coordinator;
    delete netHandler;
    /*
    FlyServer* flyServer = new FlyServer();
    if (NULL == flyServer) {
        std::cout << "error to new FlyServer!" << std::endl;
        exit(1);
    }

    // init flyServer
    flyServer->init(argc, argv);

    // 事件循环处理
    flyServer->eventMain();
    delete flyServer;
     */

}