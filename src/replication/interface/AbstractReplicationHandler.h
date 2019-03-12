//
// Created by 赵立伟 on 2019/3/6.
//

#ifndef FLYDB_ABSTRACTREPLICATIONHANDLER_H
#define FLYDB_ABSTRACTREPLICATIONHANDLER_H

#include <string>
class AbstractFlyClient;

class AbstractReplicationHandler {
public:
    virtual void unsetMaster() = 0;
    virtual const std::string &getMasterhost() const = 0;
    virtual int getMasterport() const = 0;
    virtual bool haveMasterhost() const = 0;
    virtual void setMaster(std::string ip, int port) = 0;
    virtual void cron() = 0;
    virtual void syncWithMaster(int fd,
                                std::shared_ptr<AbstractFlyClient> flyClient,
                                int mask) = 0;
    virtual void connectingStateProcess() = 0;
    virtual void recvPongStateProcess() = 0;
    virtual void sendAuthStateProcess() = 0;
    virtual void recvAuthStateProcess() = 0;
    virtual void sendPortStateProcess() = 0;
    virtual void recvPortStateProcess() = 0;
    virtual void sendIPStateProcess() = 0;
    virtual void recvIPStateProcess() = 0;
    virtual void sendCAPAStateProcess() = 0;
    virtual void recvCAPAStateProcess() = 0;
    virtual void sendPsyncStateProcess() = 0;
    virtual void recvPsyncStateProcess() = 0;
};

#endif //FLYDB_ABSTRACTREPLICATIONHANDLER_H
