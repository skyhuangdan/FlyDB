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

    SkipListNode* node = NULL;
    std::cout << "delete res :" << list->deleteNode(3, obj, &node) << std::endl;

    std::cout << "delete res :" << list->deleteNode(1, obj, &node) << std::endl;


    while(true) {
        std::cout << "flyDB> ";
        std::string command;
        std::cin >> command;
        flyServer->dealWithCommand(command);
    }
}