//
// Created by 赵立伟 on 2018/12/8.
//

#ifndef FLYDB_ABSTRACTAOFHANDLER_H
#define FLYDB_ABSTRACTAOFHANDLER_H

#include <sys/types.h>
#include "../AOFDef.h"

class AbstractAOFHandler {
public:

    virtual pid_t getChildPid() const = 0;

    virtual bool haveChildPid() const = 0;

    virtual void setChildPid(pid_t childPid) = 0;

    virtual bool IsStateOn() const = 0;

    virtual void setState(AOFState state) = 0;
};

#endif //FLYDB_ABSTRACTAOFHANDLER_H
