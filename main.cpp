#include <iostream>

#include "FlyServer.h"

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

    while(true) {
        std::cout << "flyDB> ";
        std::string command;
        std::cin >> command;
        flyServer->dealWithCommand(command);
    }
}