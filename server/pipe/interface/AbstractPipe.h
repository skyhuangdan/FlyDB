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
    virtual int open(void) = 0;

    /**
     * 设置读通道非阻塞
     */
    virtual int setReadNonBlock(void) = 0;

    /**
     * 设置写通道非阻塞
     */
    virtual int setWriteNonBlock(void) = 0;

    /**
     * 关闭管道
     **/
    virtual void closeAll(void) = 0;

    /**
     * 获取读通道fd
     */
    virtual int getReadPipe() const = 0;

    /**
     * 获取写通道fd
     */
    virtual int getWritePipe() const = 0;
};

#endif //FLYDB_ABSTRACTPIPEHANDLER_H
