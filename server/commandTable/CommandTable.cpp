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
    miscTool = MiscTool::getInstance();
}

CommandTable::~CommandTable() {
    delete this->commands;
}

int CommandTable::dealWithCommand(FlyClient* flyClient) {
    std::vector<std::string> words;
    this->miscTool->spiltString(flyClient->getQueryBuf(), " ", words);

    DictEntry* dictEntry = this->commands->findEntry(&words[0]);
    if (NULL == dictEntry) {
        sprintf(flyClient->getBuf(), "wrong command type: %s", words[0].c_str());
        return -1;
    }
    reinterpret_cast<CommandEntry*>(dictEntry->getVal())->proc(this->flyServer, flyClient, words);

    return 1;
}
