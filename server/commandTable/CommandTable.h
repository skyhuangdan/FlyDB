//
// Created by 赵立伟 on 2018/10/15.
//

#ifndef FLYDB_COMMANDTABLE_H
#define FLYDB_COMMANDTABLE_H


#include "../dataStructure/dict/Dict.h"
#include "../flyClient/FlyClient.h"
#include "../log/FileLogHandler.h"

class CommandTable {
public:
    CommandTable(const AbstractCoordinator* coordinator);
    ~CommandTable();
    int dealWithCommand(AbstractFlyClient *flyClient);

private:
    const AbstractCoordinator* coordinator;
    Dict* commands;
    AbstractLogHandler *logHandler;
};


#endif //FLYDB_COMMANDTABLE_H
