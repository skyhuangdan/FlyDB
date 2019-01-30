//
// Created by 赵立伟 on 2018/12/8.
//

#ifndef FLYDB_ABSTRACTAOFHANDLER_H
#define FLYDB_ABSTRACTAOFHANDLER_H

#include <sys/types.h>
#include "../AOFDef.h"
#include "../../io/base/Fio.h"

class AbstractCoordinator;
class AbstractAOFHandler {
public:

    virtual ~AbstractAOFHandler() {};

    virtual int start() = 0;

    virtual void backgroundSaveDone(int exitCode, int bySignal) = 0;

    virtual int rewriteBackground() = 0;

    virtual int rewriteAppendOnlyFile() = 0;

    virtual pid_t getChildPid() const = 0;

    virtual bool haveChildPid() const = 0;

    virtual void setChildPid(pid_t childPid) = 0;

    virtual bool IsStateOn() const = 0;

    virtual void setState(AOFState state) = 0;

    virtual bool isScheduled() const = 0;

    virtual void setScheduled(bool scheduled) = 0;

    virtual int saveKeyValuePair(Fio *fio,
                                 std::string key,
                                 std::shared_ptr<FlyObj> val,
                                 int64_t expireTime) = 0;
};

#endif //FLYDB_ABSTRACTAOFHANDLER_H
