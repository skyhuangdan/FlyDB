//
// Created by 赵立伟 on 2018/11/3.
//

#ifndef FLYDB_NETHANDLER_H
#define FLYDB_NETHANDLER_H

#include <sys/socket.h>
#include "../event/EventLoop.h"
#include "../utils/MiscTool.h"

class NetHandler {
public:
    static NetHandler* getInstance();
    static int setV6Only(char *err, int fd);
    static int setSendTimeout(char *err, int fd, long long ms);
    static int setTcpNoDelay(char *err, int fd, int val);               // val: 1-enbale no delay, 0-disable
    static int setSendBuffer(char *err, int fd, int buffsize);
    static int setTcpKeepAlive(char *err, int fd);
    static int resolve(char *err, char *host, char *ipbuf, size_t ipbuf_len);
    static int resolveIP(char *err, char *host, char *ipbuf, size_t ipbuf_len);
    static int createSocket(char *err, int domain);
    static int keepAlive(char *err, int fd, int interval);
    static int tcpConnect(char *err, char *addr, int port);
    static int tcpNonBlockConnect(char *err, char *addr, int port);
    static int tcpNonBlockBindConnect(char *err, char *addr, int port, char *source_addr);
    static int tcpNonBlockBestEffortBindConnect(char *err, char *addr, int port, char *source_addr);
    static int setListen(char *err, int s, struct sockaddr *sa, socklen_t len, int backlog);
    static int unixServer(char *err, const char *path, mode_t perm, int backlog);
    static int tcpServer(char *err, int port, const char *bindaddr, int backlog);
    static int tcp6Server(char *err, int port, const char *bindaddr, int backlog);
    static int setBlock(char *err, int fd, int block);
    static int tcpAccept(char *err, int s, char *ip, size_t iplen, int *port);
    static int unixAccept(char *err, int s);

    // 对应socket的绑定接口
    static void acceptTcpHandler(EventLoop *eventLoop, int fd, void *clientdata, int mask);
    static void readQueryFromClient(EventLoop *eventLoop, int fd, void *clientdata, int mask);
    static void sendReplyToClient(EventLoop *eventLoop, int fd, void *clientdata, int mask);
private:
    static void setError(char *err, const char *fmt, ...);
    static int genericResolve(char *err, char *host, char *ipbuf, size_t ipbuf_len, int flags);
    static int setReuseAddr(char *err, int fd);
    static int tcpGenericConnect(char *err, char *addr, int port, char *source_addr, int flags);
    static int tcpGenericServer(char *err, int port, const char *bindaddr, int af, int backlog);
    static int tcpGenericAccept(char *err, int s, struct sockaddr *sa, socklen_t *len);
    static void dealError(int fd, struct addrinfo *servinfo);
    static int processInputBuffer(EventLoop *eventLoop, FlyServer* flyServer, FlyClient *flyClient);
    static int processInlineBuffer(FlyClient *flyClient);
    static int processMultiBulkBuffer(FlyClient *flyClient);
    static int analyseMultiBulkLen(FlyClient *flyClient, size_t &pos);
    static int analyseBulk(FlyClient *flyClient, size_t &pos);
    static int analyseBulkLen(FlyClient *flyClient, size_t &pos);

    static MiscTool *miscTool;
};

#endif //FLYDB_NETHANDLER_H
