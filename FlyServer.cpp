//
// Created by 赵立伟 on 2018/9/18.
//

#include <iostream>
#include "FlyServer.h"
#include "CommandEntry.h"

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

    // 初始化命令表
    initCommandTable();

    return;
}

int FlyServer::getPID() {
    return this->pid;
}

FlyDB* FlyServer::getDB(int dbID) {
    return this->dbArray.at(dbID);
}

void FlyServer::initCommandTable() {
    this->commandTable["version"] = new CommandEntry(versionProc, 0);
}

std::string FlyServer::getVersion() {
    return this->version;
}

int FlyServer::dealWithCommand(std::string command) {
    auto iter = this->commandTable.find(command);
    if (iter == this->commandTable.end()) {
        std::cout << "wrong command type!" << std::endl;
        return -1;
    }

    iter->second->proc(this);
    return 1;
}


