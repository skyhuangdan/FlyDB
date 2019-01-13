//
// Created by 赵立伟 on 2019/1/13.
//

#ifndef FLYDB_ABSTRACTPIPEHANDLER_H
#define FLYDB_ABSTRACTPIPEHANDLER_H

#include "../PipeDef.h"

class AbstractPipe {
public:

    /**
     * 发送消息
     **/
    virtual void sendInfo(PipeType ptype, size_t cowSize) = 0;

    /**
     * 接收消息
     **/
    virtual PipeCowBytes* recvInfo(void) = 0;

    /**
     * 打开管道
     **/
    virtual void open(void) = 0;

    /**
     * 关闭管道
     **/
    virtual void closeAll(void) = 0;
};

#endif //FLYDB_ABSTRACTPIPEHANDLER_H
