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
    Pipe(const AbstractCoordinator *coordinator);
    ~Pipe();
    int open(void);
    void sendInfo(PipeType ptype, size_t cowSize);
    PipeCowBytes* recvInfo(void);
    void closeAll(void);

private:

    /** infoPipe[0]-read, infoPipe[1]-write */
    int infoPipe[2] = {-1, -1};
    const AbstractCoordinator *coordinator;
};


#endif //FLYDB_PIPEHANDLER_H
