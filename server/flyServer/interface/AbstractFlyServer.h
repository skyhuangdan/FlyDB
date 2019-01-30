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
    virtual ~AbstractFlyServer() {};

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

    /**
     * 当aof或者fdb子线程进行持久化的时候，可以设置canResize = true,
     * 不允许进行resize操作(除非ht.used > ht.size * NEED_FORCE_REHASH_RATIO)，
     * 这样可以减少内存搬移，以减少内存压力
     */
    virtual void updateDictResizePolicy() = 0;

    virtual int getMaxClients() const = 0;

    virtual bool isShutdownASAP() const = 0;

    virtual void setShutdownASAP(bool shutdownASAP) = 0;

    virtual int prepareForShutdown(int flags) = 0;

};

#endif //FLYDB_ABSTRACTFLYSERVER_H
