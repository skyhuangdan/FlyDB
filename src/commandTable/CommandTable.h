//
// Created by 赵立伟 on 2018/10/15.
//

#ifndef FLYDB_COMMANDTABLE_H
#define FLYDB_COMMANDTABLE_H

#include <string>
#include "../flyClient/FlyClient.h"
#include "../log/FileLogHandler.h"
#include "../dataStructure/dict/DictEntry.h"

template<class KEY, class VAL>
class Dict;

class CommandTable {
public:
    CommandTable(const AbstractCoordinator* coordinator);
    ~CommandTable();
    void populateCommand();
    int dealWithCommand(std::shared_ptr<AbstractFlyClient> flyClient);

private:
    const AbstractCoordinator* coordinator;
    Dict<std::string, CommandEntry>* commands;
    AbstractLogHandler *logHandler;
};


#endif //FLYDB_COMMANDTABLE_H
