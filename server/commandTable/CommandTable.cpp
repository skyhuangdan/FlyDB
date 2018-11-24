//
// Created by 赵立伟 on 2018/10/15.
//

#include <iostream>
#include "CommandTable.h"
#include "CommandDictType.h"
#include "CommandEntry.h"
#include "../utils/MiscTool.h"

CommandTable::CommandTable(FlyServer* flyServer) : flyServer(flyServer) {
    this->commands = new Dict(CommandDictType::getInstance());
    this->commands->addEntry(new std::string("version"), new CommandEntry(versionProc, 0));
    this->logHandler = LogHandler::getInstance();
}

CommandTable::~CommandTable() {
    delete this->commands;
}

int CommandTable::dealWithCommand(FlyClient* flyClient) {
    char *command = (char*) flyClient->getArgv()[0]->getPtr();
    DictEntry* dictEntry = this->commands->findEntry(flyClient->getArgv()[0]->getPtr());
    if (NULL == dictEntry) {
        this->logHandler->logDebug("wrong command type: %s", command);
        return -1;
    }
    reinterpret_cast<CommandEntry*>(dictEntry->getVal())->proc(this->flyServer, flyClient);

    return 1;
}
