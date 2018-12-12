//
// Created by 赵立伟 on 2018/10/15.
//

#include <iostream>
#include "CommandTable.h"
#include "CommandEntry.h"
#include "../utils/MiscTool.h"
#include "../log/FileLogFactory.h"
#include "../dataStructure/dict/Dict.cpp"

CommandTable::CommandTable(const AbstractCoordinator* coordinator) {
    this->commands = new Dict<std::string, CommandEntry>();
    this->commands->addEntry(new std::string("version"),
                             new CommandEntry(versionProc, 0));
    this->logHandler = logFactory->getLogger();
    this->coordinator = coordinator;
}

CommandTable::~CommandTable() {
    delete this->commands;
}

int CommandTable::dealWithCommand(AbstractFlyClient* flyClient) {
    std::string *command = reinterpret_cast<std::string*>
            (flyClient->getArgv()[0]->getPtr());
    DictEntry<std::string, CommandEntry>* dictEntry =
            this->commands->findEntry(command);
    if (NULL == dictEntry) {
        this->logHandler->logDebug("wrong command type: %s", command);
        return -1;
    }
    dictEntry->getVal()->proc(
            this->coordinator->getFlyServer(), flyClient);

    return 1;
}
