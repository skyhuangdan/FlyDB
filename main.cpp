#include <iostream>

#include "server/FlyServer.h"
#include "server/dataStructure/dict/Dict.h"
#include "server/dataStructure/dict/test/TestDictType.h"
#include "server/dataStructure/skiplist/SkipList.h"
#include "server/dataStructure/skiplist/test/TestSkipListType.h"
#include "server/dataStructure/intset/IntSet.h"
#include "server/utils/MiscTool.h"

/**
 * flyDB，取名fly有两层含义：
 *  第一："飞"谐音"菲"，以爱妻命名
 *  第二："飞"寓意飞快，代表其高性能
 * created by zlw, 20180918
 */

int main(int argc, char **argv) {
    std::cout << "Hello, flyDB. Wish you be better!" << std::endl;

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
}