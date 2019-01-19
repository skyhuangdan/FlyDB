//
// Created by 赵立伟 on 2018/12/3.
//

#ifndef FLYDB_ABSTRACTFLYSERVER_H
#define FLYDB_ABSTRACTFLYSERVER_H

#include <string>
#include "../../net/interface/AbstractNetHandler.h"
#include "../../db/interface/AbstractFlyDB.h"
#include "../../config/ConfigCache.h"
#include "../FlyServerDef.h"

class AbstractFlyServer {
public:
    // 初始化函数
    virtual void init(ConfigCache *configCache) = 0;

    // 获取server id
    virtual pid_t getPID() = 0;

    // 获取版本号
    virtual std::string getVersion() = 0;

    // 处理命令
    virtual int dealWithCommand(AbstractFlyClient *flyclient) = 0;

    virtual int getHz() const = 0;

    virtual void setHz(int hz) = 0;

    virtual char *getNeterr() const = 0;

    virtual AbstractFlyClient *createClient(int fd) = 0;

    virtual int deleteClient(int fd) = 0;

    virtual time_t getNowt() const = 0;

    virtual void setNowt(time_t nowt) = 0;

    virtual size_t getClientMaxQuerybufLen() const = 0;

    virtual int64_t getStatNetInputBytes() const = 0;

    virtual void addToStatNetInputBytes(int64_t size) = 0;

    virtual void closeListeningSockets(bool unlinkUnixSocket) = 0;

    virtual void addToClientsPendingToWrite(AbstractFlyClient *flyClient) = 0;

    virtual int handleClientsWithPendingWrites() = 0;

    virtual void freeClientAsync(AbstractFlyClient *flyClient) = 0;

    virtual void freeClientsInAsyncFreeList() = 0;

    virtual AbstractFlyDB *getFlyDB(int dbnum) = 0;

    virtual uint8_t getFlyDBCount() const = 0;

    virtual int getMaxClients() const = 0;

    virtual bool isShutdownASAP() const = 0;

    virtual void setShutdownASAP(bool shutdownASAP) = 0;

    virtual pid_t getFdbChildPid() const = 0;
    
    virtual bool haveFdbChildPid() const = 0;

    virtual void setFdbChildPid(pid_t fdbChildPid) = 0;

    virtual bool isFdbBGSaveScheduled() const = 0;

    virtual void setFdbBGSaveScheduled(bool fdbBGSaveScheduled) = 0;

    virtual bool canBgsaveNow() = 0;

    virtual time_t getLastSaveTime() const = 0;

    virtual bool lastSaveTimeGapGreaterThan(time_t gap) const = 0;

    virtual void setLastSaveTime(time_t lastSaveTime) = 0;

    virtual void setBgsaveLastTryTime(time_t bgsaveLastTryTime) = 0;

    virtual int getLastBgsaveStatus() const = 0;

    virtual void setLastBgsaveStatus(int lastBgsaveStatus) = 0;

    virtual int getFdbChildType() const = 0;

    virtual void setFdbDiskChildType() = 0;

    virtual void setFdbNoneChildType() = 0;

    virtual void setFdbBgSaveDone(int status) = 0;

    virtual void setFdbSaveDone() = 0;

    virtual int getSaveParamsCount() const = 0;

    virtual const saveParam* getSaveParam(int pos) const = 0;

    virtual uint64_t getDirty() const = 0;

    virtual uint64_t addDirty(uint64_t count) = 0;

    virtual pid_t getAofChildPid() const = 0;
    
    virtual bool haveAofChildPid() const = 0;

    virtual void setAofChildPid(pid_t aofChildPid) = 0;
};

#endif //FLYDB_ABSTRACTFLYSERVER_H
