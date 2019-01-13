//
// Created by 赵立伟 on 2019/1/13.
//

#ifndef FLYDB_PIPEHANDLER_H
#define FLYDB_PIPEHANDLER_H

#include "interface/AbstractPipe.h"
#include "../coordinator/interface/AbstractCoordinator.h"

class Pipe : public AbstractPipe {
public:
    Pipe(const AbstractCoordinator *coordinator);
    void open();

};


#endif //FLYDB_PIPEHANDLER_H
