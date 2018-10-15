//
// Created by 赵立伟 on 2018/9/19.
//

#include <iostream>
#include "../FlyServer.h"
#include "CommandEntry.h"

CommandEntry::CommandEntry(commandProc proc, int flag) :
        proc(proc), flag(flag) {}

void versionProc(FlyServer* server) {
    if (NULL == server) {
        return;
    }

    std::cout << "FlyDB version: " << server->getVersion() << std::endl;
}
