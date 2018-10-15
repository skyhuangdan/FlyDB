//
// Created by 赵立伟 on 2018/10/15.
//

#include <iostream>
#include "CommandTable.h"
#include "CommandDictType.h"
#include "CommandEntry.h"

CommandTable::CommandTable(FlyServer* flyServer) : flyServer(flyServer) {
    this->commands = new Dict(CommandDictType::getInstance());
    this->commands->addEntry(new std::string("version"), new CommandEntry(versionProc, 0));
}

int CommandTable::dealWithCommand(std::string* command) {
    DictEntry* dictEntry = this->commands->findEntry(command);
    if (NULL == dictEntry) {
        std::cout << "wrong command type!" << std::endl;
        return -1;
    }
    reinterpret_cast<CommandEntry*>(dictEntry->getVal())->proc(this->flyServer);

    return 1;
}
