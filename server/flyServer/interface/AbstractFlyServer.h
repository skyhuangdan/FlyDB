//
// Created by 赵立伟 on 2018/12/3.
//

#ifndef FLYDB_ABSTRACTFLYSERVER_H
#define FLYDB_ABSTRACTFLYSERVER_H

#include <string>
#include "../../net/interface/AbstractNetHandler.h"

class AbstractFlyDB;

class AbstractFlyServer {
public:
    // 初始化函数
    virtual void init(int argc, char **argv) = 0;

    // 获取server id
    virtual int getPID() = 0;

    // 获取版本号
    virtual std::string getVersion() = 0;

    // 处理命令
    virtual int dealWithCommand(AbstractFlyClient *flyclient) = 0;

    // 事件循环处理
    virtual void eventMain() = 0;

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

    virtual int getVerbosity() const = 0;

    virtual char *getLogfile() const = 0;

    virtual int getSyslogEnabled() const = 0;

    virtual char *getSyslogIdent() const = 0;

    virtual int getSyslogFacility() const = 0;

    virtual AbstractNetHandler *getNetHandler() const = 0;

    virtual void addToClientsPendingToWrite(AbstractFlyClient *flyClient) = 0;

    virtual int handleClientsWithPendingWrites() = 0;

    virtual void freeClientAsync(AbstractFlyClient *flyClient) = 0;

    virtual void freeClientsInAsyncFreeList() = 0;

    virtual AbstractFlyDB *getFlyDB(int dbnum) = 0;

    virtual uint8_t getFlyDBCount() const = 0;
};

#endif //FLYDB_ABSTRACTFLYSERVER_H
