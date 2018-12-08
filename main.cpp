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

    AbstractCoordinator *coordinator = new Coordinator();
    coordinator->getEventLoop()->eventMain();
    delete coordinator;
}