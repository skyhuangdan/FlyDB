//
// Created by 赵立伟 on 2018/9/19.
//

#ifndef FLYDB_COMMANDENTRY_H
#define FLYDB_COMMANDENTRY_H

#include "../flyClient/interface/AbstractFlyClient.h"
#include "../flyServer/interface/AbstractFlyServer.h"

class FlyServer;
typedef void (*commandProc)(AbstractFlyServer*, AbstractFlyClient*);

struct CommandEntry {
    CommandEntry(commandProc proc, int flag);

    commandProc proc;
    int flag;
};

void versionProc(AbstractFlyServer*, AbstractFlyClient*);

#endif //FLYDB_COMMANDENTRY_H
