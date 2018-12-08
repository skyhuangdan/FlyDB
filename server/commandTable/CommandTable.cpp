//
// Created by 赵立伟 on 2018/10/15.
//

#include <iostream>
#include "CommandTable.h"
#include "CommandDictType.h"
#include "CommandEntry.h"
#include "../utils/MiscTool.h"
#include "../log/FileLogFactory.h"

CommandTable::CommandTable(const AbstractCoordinator* coordinator) {
    this->commands = new Dict(CommandDictType::getInstance());
    this->commands->addEntry(new std::string("version"),
                             new CommandEntry(versionProc, 0));
    this->logHandler = logFactory->getLogger();
    this->coordinator = coordinator;
}

CommandTable::~CommandTable() {
    delete this->commands;
}

int CommandTable::dealWithCommand(AbstractFlyClient* flyClient) {
    char *command = reinterpret_cast<char*>(flyClient->getArgv()[0]->getPtr());
    DictEntry* dictEntry = this->commands->findEntry(
            flyClient->getArgv()[0]->getPtr());
    if (NULL == dictEntry) {
        this->logHandler->logDebug("wrong command type: %s", command);
        return -1;
    }
    reinterpret_cast<CommandEntry*>(dictEntry->getVal())->proc(
            this->coordinator->getFlyServer(), flyClient);

    return 1;
}
