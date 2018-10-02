#include <iostream>

#include "server/FlyServer.h"
#include "server/dataStructure/dict/Dict.h"
#include "server/dataStructure/dict/test/TestDictType.h"
#include "server/dataStructure/skiplist/SkipList.h"
#include "server/dataStructure/skiplist/test/TestSkipListType.h"

/**
 * flyDB，取名fly有两层含义：
 *  第一："飞"谐音"菲"，以爱妻命名
 *  第二："飞"寓意飞快，代表其高性能
 * created by zlw, 20180918
 */

int main() {
    std::cout << "Hello, flyDB. Wish you be better!" << std::endl;

    FlyServer* flyServer = new FlyServer();
    if (NULL == flyServer) {
        std::cout << "error to new FlyServer!" << std::endl;
        exit(1);
    }

    flyServer->init();

    /*
    Dict* dict = new Dict(TestDictType());
    char *key = "key";
    char *val = "val";
    for (int i = 0; i < 5; i++) {
        dict->addEntry((void*)key, (void*)val);
    }
    unsigned long cursor = dict->dictScan(0, 5, testScanProc, NULL);
    std::cout << "next cursor: " << cursor << std::endl;
     */

    SkipList* list = new SkipList(TestSkipListType());
    void * obj = new std::string("123");
    list->insertNode(1, obj);
    list->insertNode(5, obj);
    list->insertNode(4, obj);
    list->insertNode(3, obj);
    list->insertNode(2, obj);
    list->insertNode(9, obj);
    list->insertNode(8, obj);
    list->insertNode(6, obj);
    list->insertNode(19, obj);
    list->insertNode(18, obj);
    list->insertNode(16, obj);
    list->insertNode(14, obj);
    list->insertNode(13, obj);
    list->insertNode(12, obj);

    SkipListRange range = SkipListRange(-5, 25, false, false);
    SkipListNode *node = list->firstInRange(range);
    if (node != NULL) {
        std::cout << "the first node val = " << node->getScore() << std::endl;
    } else {
        std::cout << "the first node is NULL" << std::endl;
    }

    node = list->lastInRange(range);
    if (node != NULL) {
        std::cout << "the last node val = " << node->getScore() << std::endl;
    } else {
        std::cout << "the last node is NULL" << std::endl;
    }

    while(true) {
        std::cout << "flyDB> ";
        std::string command;
        std::cin >> command;
        flyServer->dealWithCommand(command);
    }
}