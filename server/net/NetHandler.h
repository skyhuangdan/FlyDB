//
// Created by 赵立伟 on 2018/11/3.
//

#ifndef FLYDB_NETHANDLER_H
#define FLYDB_NETHANDLER_H

#include <sys/socket.h>
#include "../event/EventLoop.h"
#include "../utils/MiscTool.h"
#include "interface/AbstractNetHandler.h"
#include "../log/interface/AbstractLogHandler.h"
#include "../log/interface/AbstractLogFactory.h"

// 对应socket的绑定接口
void acceptTcpHandler(const AbstractCoordinator *coordinator, int fd, void *clientdata, int mask);
void readQueryFromClient(const AbstractCoordinator *coordinator, int fd, void *clientdata, int mask);
void sendReplyToClient(const AbstractCoordinator *coordinator, int fd, void *clientdata, int mask);

class NetHandler : public AbstractNetHandler {
public:
    static NetHandler* getInstance();
    int setV6Only(char *err, int fd);
    int setSendTimeout(char *err, int fd, long long ms);
    int setTcpNoDelay(char *err, int fd, int val);               // val: 1-enbale no delay, 0-disable
    int setSendBuffer(char *err, int fd, int buffsize);
    int setTcpKeepAlive(char *err, int fd);
    int resolve(char *err, char *host, char *ipbuf, size_t ipbuf_len);
    int resolveIP(char *err, char *host, char *ipbuf, size_t ipbuf_len);
    int createSocket(char *err, int domain);
    int keepAlive(char *err, int fd, int interval);
    int tcpConnect(char *err, char *addr, int port);
    int tcpNonBlockConnect(char *err, char *addr, int port);
    int tcpNonBlockBindConnect(char *err, char *addr, int port, char *source_addr);
    int tcpNonBlockBestEffortBindConnect(char *err, char *addr, int port, char *source_addr);
    int setListen(char *err, int s, struct sockaddr *sa, socklen_t len, int backlog);
    int unixServer(char *err, const char *path, mode_t perm, int backlog);
    int tcpServer(char *err, int port, const char *bindaddr, int backlog);
    int tcp6Server(char *err, int port, const char *bindaddr, int backlog);
    int setBlock(char *err, int fd, int block);
    int tcpAccept(char *err, int s, char *ip, size_t iplen, int *port);
    int unixAccept(char *err, int s);
    int processInputBuffer(const AbstractCoordinator* coordinator,
                           AbstractFlyClient *flyClient);
    int writeToClient(const AbstractCoordinator *coordinator,
                      AbstractFlyClient *flyClient,
                      int handlerInstalled);

private:
    NetHandler();
    void setError(char *err, const char *fmt, ...);
    int genericResolve(char *err, char *host, char *ipbuf, size_t ipbuf_len, int flags);
    int setReuseAddr(char *err, int fd);
    int tcpGenericConnect(char *err, char *addr, int port, char *source_addr, int flags);
    int tcpGenericServer(char *err, int port, const char *bindaddr, int af, int backlog);
    int tcpGenericAccept(char *err, int s, struct sockaddr *sa, socklen_t *len);
    void dealError(int fd, struct addrinfo *servinfo);
    int processInlineBuffer(AbstractFlyClient *flyClient);
    int processMultiBulkBuffer(AbstractFlyClient *flyClient);
    int analyseMultiBulkLen(AbstractFlyClient *flyClient, size_t &pos);
    int analyseMultiBulk(AbstractFlyClient *flyClient, size_t &pos);
    int analyseBulk(AbstractFlyClient *flyClient);
    int setProtocolError(char *err, AbstractFlyClient *flyClient, size_t pos);
    void addReplyErrorFormat(AbstractFlyClient *flyClient, const char *fmt, ...);
    int addReplyError(AbstractFlyClient *flyClient, const char *err);

    MiscTool *miscTool;
    AbstractLogHandler *logHandler;
};

#endif //FLYDB_NETHANDLER_H
