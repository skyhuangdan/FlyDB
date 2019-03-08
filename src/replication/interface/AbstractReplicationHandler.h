//
// Created by 赵立伟 on 2019/3/6.
//

#ifndef FLYDB_ABSTRACTREPLICATIONHANDLER_H
#define FLYDB_ABSTRACTREPLICATIONHANDLER_H

#include <string>

class AbstractReplicationHandler {
public:
    virtual void unsetMaster() = 0;
    virtual const std::string &getMasterhost() const = 0;
    virtual int getMasterport() const = 0;
    virtual bool haveMasterhost() const = 0;
};

#endif //FLYDB_ABSTRACTREPLICATIONHANDLER_H
