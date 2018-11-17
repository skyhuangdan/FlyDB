//
// Created by 赵立伟 on 2018/10/15.
//

#ifndef FLYDB_COMMANDTABLE_H
#define FLYDB_COMMANDTABLE_H


#include "../dataStructure/dict/Dict.h"
#include "../flyClient/FlyClient.h"

class FlyServer;
class MiscTool;
class CommandTable {
public:
    CommandTable(FlyServer* flyServer);
    ~CommandTable();
    int dealWithCommand(FlyClient *flyClient);

private:
    FlyServer* flyServer;
    Dict* commands;
    MiscTool *miscTool;
};


#endif //FLYDB_COMMANDTABLE_H
