#include <iostream>

#include "src/def.h"
#include "src/coordinator/Coordinator.h"
#include "src/fdb/FDBHandler.h"

/**
 * flyDB，取名fly有两层含义：
 *  第一："飞"谐音"菲"
 *  第二："飞"寓意飞快，代表其高性能
 * created by zlw, 20180918
 */

AbstractLogFactory *logFactory = new FileLogFactory();
MiscTool *miscTool = MiscTool::getInstance();
AbstractCoordinator *coordinator = NULL;

int main(int argc, char **argv) {
    std::cout << "Hello, flyDB. Wish you be better!" << std::endl;

    /** 初始化随机种子 */
    srand((unsigned)time(NULL) ^ getpid());

    coordinator = new Coordinator();
    coordinator->getEventLoop()->eventMain();

    delete coordinator;
}