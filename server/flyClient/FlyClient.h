//
// Created by 赵立伟 on 2018/10/18.
//

#ifndef FLYDB_FLYCLIENT_H
#define FLYDB_FLYCLIENT_H


#include <string>
#include <list>
#include "../dataStructure/flyObj/FlyObj.h"
#include "../commandTable/CommandEntry.h"

const int FLY_REPLY_CHUNK_BYTES = 16 * 1024;

class FlyClient {
public:
    FlyClient(int fd);
    ~FlyClient();
    uint64_t getId() const;
    void setId(uint64_t id);
    int getFd() const;
    void setFd(int fd);
    FlyObj *getName() const;
    void setName(FlyObj *name);
    int getFlags() const;
    void setFlags(int flags);
    const std::string &getQueryBuf() const;
    void setQueryBuf(const std::string &queryBuf);
    void addToQueryBuf(const std::string &str);             // 向输入缓冲中添加数据
    int getQueryBufSize() const;
    FlyObj **getArgv() const;
    void freeArgv() const;
    void setArgv(int64_t multiBulkLen);
    int getArgc() const;
    void setArgc(int argc);
    CommandEntry *getCmd() const;
    void setCmd(CommandEntry *cmd);
    char *getBuf() const;
    void setBuf(char *buf);
    const std::list<std::string> &getReply() const;
    void setReply(const std::list<std::string> &reply);
    int getAuthentiated() const;
    void setAuthentiated(int authentiated);
    time_t getCreateTime() const;
    void setCreateTime(time_t createTime);
    time_t getLastInteractionTime() const;
    void setLastInteractionTime(time_t lastInteractionTime);
    time_t getSoftLimitTime() const;
    void setSoftLimitTime(time_t softLimitTime);
    bool isMultiBulkType();
    int getMultiBulkLen() const;
    void setMultiBulkLen(int multiBulkLen);

private:
    uint64_t id;
    int fd;                             // 套接字
    FlyObj* name;                       // client名字
    int flags;                          // 标志
    std::string queryBuf;               // 输入缓冲区
    FlyObj **argv;                      // 命令参数
    int argc;
    CommandEntry* cmd;                  // 命令实现函数
    char *buf;                          // 固定大小输出缓冲区
    std::list<std::string> reply;       // 可变长度输出缓冲区
    int authentiated;                   // 是否通过了身份验证
    time_t createTime;                  // 客户端创建事件
    time_t lastInteractionTime;         // 最后一次互动时间
    time_t softLimitTime;               // 软性限制时间
    int reqType;
    int multiBulkLen;                   // 剩余可读的multi bulk参数数量
};


#endif //FLYDB_FLYCLIENT_H
