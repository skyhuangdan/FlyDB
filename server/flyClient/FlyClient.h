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
    FlyObj **getArgv() const;
    void setArgv(FlyObj **argv);
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

private:
    uint64_t id;
    // 套接字
    int fd;
    // client名字
    FlyObj* name;
    // 标志
    int flags;
    // 输入缓冲区
    std::string queryBuf;
    // 命令参数
    FlyObj **argv;
    int argc;
    // 命令实现函数
    CommandEntry* cmd;
    // 固定大小输出缓冲区
    char *buf;
    // 可变长度输出缓冲区
    std::list<std::string> reply;
    // 是否通过了身份验证
    int authentiated;
    // 客户端创建事件
    time_t createTime;
    // 最后一次互动时间
    time_t lastInteractionTime;
    // 软性限制时间
    time_t softLimitTime;
};


#endif //FLYDB_FLYCLIENT_H
