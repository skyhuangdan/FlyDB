//
// Created by 赵立伟 on 2018/12/8.
//

#ifndef FLYDB_ABSTRACTAOFHANDLER_H
#define FLYDB_ABSTRACTAOFHANDLER_H

#include <sys/types.h>
#include "../AOFDef.h"

class AbstractAOFHandler {
public:

    virtual pid_t getAofChildPid() const = 0;

    virtual bool haveAofChildPid() const = 0;

    virtual void setAofChildPid(pid_t aofChildPid) = 0;

    virtual bool IsAofStateOn() const = 0;

    virtual void setAofState(AOFState aofState) = 0;
};

#endif //FLYDB_ABSTRACTAOFHANDLER_H
