//
// Created by 赵立伟 on 2018/10/15.
//

#ifndef FLYDB_COMMANDTABLE_H
#define FLYDB_COMMANDTABLE_H


#include "../dataStructure/dict/Dict.h"
#include "../flyClient/FlyClient.h"
#include "../log/LogFileHandler.h"

class FlyServer;
class MiscTool;
class AbstractLogHandler;
class CommandTable {
public:
    CommandTable(FlyServer* flyServer);
    ~CommandTable();
    int dealWithCommand(AbstractFlyClient *flyClient);

private:
    FlyServer* flyServer;
    Dict* commands;
    AbstractLogHandler *logHandler;
};


#endif //FLYDB_COMMANDTABLE_H
