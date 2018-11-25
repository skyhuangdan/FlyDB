//
// Created by 赵立伟 on 2018/11/18.
//

#ifndef FLYDB_CLIENTDEF_H
#define FLYDB_CLIENTDEF_H

const int CLIENT_CLOSE_AFTER_REPLY = 1 << 6;     // 回复后关闭(即处理完该请求后关闭)
const int CLIENT_PENDING_WRITE = 1 << 21;        // client中有需要发送的数据，但是write handler还没有install
const int CLIENT_CLOSE_ASAP = 1 << 10;           // 异步地关闭该client
const int FLY_REPLY_CHUNK_BYTES = 16 * 1024;

#endif //FLYDB_CLIENTDEF_H
