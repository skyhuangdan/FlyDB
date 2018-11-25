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

    char buf[1024];
    sprintf(buf, "FlyDB version: %s", server->getVersion().c_str());
    client->addReply(buf, strlen(buf));
}
