//
// Created by 赵立伟 on 2018/9/19.
//

#ifndef FLYDB_COMMANDENTRY_H
#define FLYDB_COMMANDENTRY_H

class FlyServer;
class FlyClient;
typedef void (*commandProc)(FlyServer*, FlyClient*);

struct CommandEntry {
    CommandEntry(commandProc proc, int flag);

    commandProc proc;
    int flag;
};

void versionProc(FlyServer*, FlyClient*);

#endif //FLYDB_COMMANDENTRY_H
