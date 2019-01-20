//
// Created by 赵立伟 on 2018/11/29.
//

#ifndef FLYDB_AOFHANDLER_H
#define FLYDB_AOFHANDLER_H

#include "interface/AbstractAOFHandler.h"
#include "../coordinator/interface/AbstractCoordinator.h"

class AOFHandler : public AbstractAOFHandler {
public:
    AOFHandler(AbstractCoordinator *coordinator,
               char *fileName,
               AOFState state);
    pid_t getChildPid() const;
    void setChildPid(pid_t childPid);
    bool haveChildPid() const;
    bool IsStateOn() const;
    void setState(AOFState aofState);

private:
    AbstractCoordinator *coordinator;
    char *fileName;
    AOFState state;
    pid_t childPid = -1;
    std::string aofBuf;


};


#endif //FLYDB_AOFHANDLER_H
