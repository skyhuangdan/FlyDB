//
// Created by 赵立伟 on 2018/11/3.
//

#ifndef FLYDB_NETHANDLER_H
#define FLYDB_NETHANDLER_H

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
    static int anetCreateSocket(char *err, int domain);
    static int keepAlive(char *err, int fd, int interval);
    static int tcpConnect(char *err, char *addr, int port);
    static int tcpNonBlockConnect(char *err, char *addr, int port);
    static int tcpNonBlockBindConnect(char *err, char *addr, int port, char *source_addr);
    static int tcpNonBlockBestEffortBindConnect(char *err, char *addr, int port, char *source_addr);
private:
    static void setError(char *err, const char *fmt, ...);
    static int genericResolve(char *err, char *host, char *ipbuf, size_t ipbuf_len, int flags);
    static int setReuseAddr(char *err, int fd);
    static int tcpGenericConnect(char *err, char *addr, int port, char *source_addr, int flags);
    static int setBlock(char *err, int fd, int non_block);
};


#endif //FLYDB_NETHANDLER_H
