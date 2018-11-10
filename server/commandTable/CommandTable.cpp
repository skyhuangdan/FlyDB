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
}

CommandTable::~CommandTable() {
    delete this->commands;
}

int CommandTable::dealWithCommand(FlyClient* flyClient) {
    std::vector<std::string> words;
    MiscTool::spiltString(flyClient->getQueryBuf(), " ", words);

    DictEntry* dictEntry = this->commands->findEntry((void*) words[0].c_str());
    if (NULL == dictEntry) {
        std::cout << "wrong command type!" << std::endl;
        return -1;
    }
    reinterpret_cast<CommandEntry*>(dictEntry->getVal())->proc(this->flyServer, flyClient, words);

    return 1;
}
