//
// Created by 赵立伟 on 2018/9/19.
//

#include <iostream>
#include "../FlyServer.h"
#include "CommandEntry.h"

CommandEntry::CommandEntry(commandProc proc, int flag) :
        proc(proc), flag(flag) {}

void versionProc(FlyServer* server, FlyClient *client, std::vector<std::string> &words) {
    if (NULL == server || NULL == client) {
        return;
    }

    sprintf(client->getBuf(), "FlyDB version: %s", server->getVersion().c_str());

    std::cout << "FlyDB version: " << server->getVersion() << std::endl;
}
