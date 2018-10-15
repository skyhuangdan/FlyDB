//
// Created by 赵立伟 on 2018/9/18.
//

#include <iostream>
#include "FlyServer.h"
#include "commandTable/CommandEntry.h"

FlyServer::FlyServer() {
}

void FlyServer::init() {
    // init db array
    for (int i = 0; i < DB_NUM; i++) {
        this->dbArray[i] = new FlyDB();
        if (NULL == this->dbArray.at(i)) {
            std::cout << "error to create FlyDB[" << i << "]" << std::endl;
            exit(1);
        }
    }

    // init command table
    commandTable = new CommandTable(this);

    return;
}

int FlyServer::getPID() {
    return this->pid;
}

FlyDB* FlyServer::getDB(int dbID) {
    return this->dbArray.at(dbID);
}

std::string FlyServer::getVersion() {
    return this->version;
}

int FlyServer::dealWithCommand(std::string* command) {
    return this->commandTable->dealWithCommand(command);
}


