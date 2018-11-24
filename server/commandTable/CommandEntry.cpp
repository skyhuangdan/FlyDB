//
// Created by 赵立伟 on 2018/9/19.
//

#include <iostream>
#include "../FlyServer.h"
#include "CommandEntry.h"

CommandEntry::CommandEntry(commandProc proc, int flag) :
        proc(proc), flag(flag) {}

void versionProc(FlyServer* server, FlyClient *client) {
    if (NULL == server || NULL == client) {
        return;
    }

    sprintf((char*)client->getBuf(), "FlyDB version: %s", server->getVersion().c_str());
}
