//
// Created by 赵立伟 on 2018/11/29.
//

#ifndef FLYDB_AOFHANDLER_H
#define FLYDB_AOFHANDLER_H

#include "interface/AbstractAOFHandler.h"
#include "../coordinator/interface/AbstractCoordinator.h"

class AOFHandler : public AbstractAOFHandler {
public:
    AOFHandler(AbstractCoordinator *coordinator, char *fileName, AOFState aofState);
    pid_t getAofChildPid() const;
    void setAofChildPid(pid_t aofChildPid);
    bool haveAofChildPid() const;
    bool IsAofStateOn() const;
    void setAofState(AOFState aofState);

private:
    AbstractCoordinator *coordinator;
    char *fileName;
    AOFState aofState;
    pid_t aofChildPid = -1;
    std::string aofBuf;


};


#endif //FLYDB_AOFHANDLER_H
