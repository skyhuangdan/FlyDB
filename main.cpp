#include <iostream>

#include "def.h"
#include "server/coordinator/Coordinator.h"
#include "server/fdb/FDBHandler.h"

/**
 * flyDB，取名fly有两层含义：
 *  第一："飞"谐音"菲"
 *  第二："飞"寓意飞快，代表其高性能
 * created by zlw, 20180918
 */

AbstractLogFactory *logFactory = new FileLogFactory();
MiscTool *miscTool = MiscTool::getInstance();

int main(int argc, char **argv) {
    std::cout << "Hello, flyDB. Wish you be better!" << std::endl;

    AbstractCoordinator *coordinator = new Coordinator();
    FDBSaveInfo fdbSaveInfo = FDBSaveInfo();
    coordinator->getFdbHandler()->load(fdbSaveInfo);

    coordinator->getEventLoop()->eventMain();

    delete coordinator;
}