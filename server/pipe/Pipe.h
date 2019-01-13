//
// Created by 赵立伟 on 2019/1/13.
//

#ifndef FLYDB_PIPEHANDLER_H
#define FLYDB_PIPEHANDLER_H

#include "interface/AbstractPipe.h"
#include "../coordinator/interface/AbstractCoordinator.h"
#include "PipeDef.h"

class Pipe : public AbstractPipe {
public:
    Pipe(const AbstractCoordinator *coordinator, int infoPipe[2]);
    ~Pipe();
    void sendInfo(PipeType ptype);
    PipeCowBytes* recvInfo(void);

private:
    void open(void);
    void closeAll(void);

    int infoPipe[2];
    const AbstractCoordinator *coordinator;
};


#endif //FLYDB_PIPEHANDLER_H
