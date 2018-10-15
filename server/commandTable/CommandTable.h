//
// Created by 赵立伟 on 2018/10/15.
//

#ifndef FLYDB_COMMANDTABLE_H
#define FLYDB_COMMANDTABLE_H


#include "../dataStructure/dict/Dict.h"

class FlyServer;
class CommandTable {
public:
    CommandTable(FlyServer* flyServer);
    int dealWithCommand(std::string* command);

private:
    FlyServer* flyServer;
    Dict* commands;

};


#endif //FLYDB_COMMANDTABLE_H
