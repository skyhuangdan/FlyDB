//
// Created by 赵立伟 on 2018/9/18.
//

#include "FlyServer.h"

FlyServer::FlyServer() {

}

void FlyServer::init() {
    for (int i = 0; i < DB_NUM; i++) {
        this->dbArray[i] = new FlyDB();
        if (NULL == this->dbArray.at(i)) {
            printf("error to create FlyDB[%d]", i);
            exit(0);
        }
    }

    return;
}

pid_t FlyServer::getPID() {
    return this->pid;
}

FlyDB* FlyServer::getDB(int dbID) {
    return this->dbArray.at(dbID);
}

