//
// Created by 赵立伟 on 2018/11/3.
//

#include <sys/socket.h>
#include <sys/time.h>
#include <string>
#include <errno.h>
#include <netinet/in.h>
#include <cstdarg>
#include <cstdio>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <zconf.h>
#include <fcntl.h>
#include <array>
#include <sys/un.h>
#include <sys/stat.h>
#include <iostream>
#include <poll.h>
#include "NetHandler.h"
#include "NetDef.h"
#include "../../def.h"
#include "../utils/MiscTool.h"
#include "../flyClient/ClientDef.h"
#include "../log/FileLogHandler.h"
#include "../flyObj/FlyObj.h"
#include "../log/FileLogFactory.h"
#include "../flyObj/FlyObjString/FlyObjString.h"

NetHandler::NetHandler() {
    this->logHandler = logFactory->getLogger();
}

NetHandler::~NetHandler() {

}

NetHandler* NetHandler::getInstance() {
    static NetHandler* instance;
    if (NULL == instance) {
        instance = new NetHandler();
    }

    return instance;
}

int NetHandler::setV6Only(char *err, int fd) {
    int yes = 1;

    if (-1 == setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &yes, sizeof(yes))) {
        setError(err, "setsockopt IPV6_V6ONLY: %s", strerror(errno));
        return -1;
    }
    return 1;
}

int NetHandler::setSendTimeout(char *err,
                               int fd,
                               long long ms) {
    struct timeval tv;
    tv.tv_sec = ms / 1000;
    tv.tv_usec = ms % 1000;

    if (-1 == setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv))) {
        setError(err, "setsockopt SO_SNDTIMEO: %s", strerror(errno));
        return -1;
    }
    return 1;
}

int NetHandler::setTcpNoDelay(char *err, int fd, int val) {
    if (-1 == setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val))) {
        setError(err, "setsockopt TCP_NODELAY: %s", strerror(errno));
        return -1;
    }
    return 1;
}

int NetHandler::setSendBuffer(char *err, int fd, int buffsize) {
    if (-1 == setsockopt(fd, SOL_SOCKET, SO_SNDBUF,
                         &buffsize, sizeof(buffsize))) {
        setError(err, "setsockopt SO_SNDBUF: %s", strerror(errno));
        return -1;
    }
    return 1;
}

int NetHandler::setTcpKeepAlive(char *err, int fd) {
    int yes;
    if (-1 == setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(yes))) {
        setError(err, "setsockopt SO_KEEPALIVE: %s", strerror(errno));
        return -1;
    }
    return 1;
}

int NetHandler::resolve(char *err,
                        char *host,
                        char *ipbuf,
                        size_t ipbuf_len) {
    return genericResolve(err, host, ipbuf, ipbuf_len, NET_NONE);
}

int NetHandler::resolveIP(char *err,
                          char *host,
                          char *ipbuf,
                          size_t ipbuf_len) {
    return genericResolve(err, host, ipbuf, ipbuf_len, NET_IP_ONLY);
}

int NetHandler::createSocket(char *err, int domain) {
    int sock;
    if ((sock = socket(domain, SOCK_STREAM, 0)) == -1) {
        setError(err, "creating socket: %s", strerror(errno));
        return -1;
    }

    if (-1 == setReuseAddr(err, sock)) {
        close(sock);
        return -1;
    }
    return sock;
}

int NetHandler::keepAlive(char *err, int fd, int interval) {
    int val = 1;

    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val)) == -1) {
        setError(err, "setsockopt SO_KEEPALIVE: %s", strerror(errno));
        return -1;
    }

#ifdef __linux__
    // 这些都是linux特有api，所以只有linux环境下才会执行如下步骤
    // 开始首次keepAlive探测前的TCP空闲时间
    val = interval;
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &val, sizeof(val)) < 0) {
        setError(err, "setsockopt TCP_KEEPIDLE: %s\n", strerror(errno));
        return -1;
    }

    // 两次keepAlive探测间的间隔时间
    val = interval / 3;
    if (val == 0) {
        val = 1;
    }
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &val, sizeof(val)) < 0) {
        setError(err, "setsockopt TCP_KEEPINTVL: %s\n", strerror(errno));
        return -1;
    }

    // 判定断开前的keepAlive探测次数
    val = 3;
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &val, sizeof(val)) < 0) {
        setError(err, "setsockopt TCP_KEEPCNT: %s\n", strerror(errno));
        return -1;
    }
#else
    ((void) interval); /* Avoid unused var warning for non Linux systems. */
#endif

    return 1;
}

int NetHandler::tcpConnect(char *err, char *addr, int port) {
    return tcpGenericConnect(err, addr, port, NULL, NET_CONNECT_NONE);
}

int NetHandler::tcpNonBlockConnect(char *err, char *addr, int port) {
    return tcpGenericConnect(err, addr, port, NULL, NET_CONNECT_NONBLOCK);
}

int NetHandler::tcpNonBlockBindConnect(char *err,
                                       char *addr,
                                       int port,
                                       char *source_addr) {
    return tcpGenericConnect(err, addr, port, source_addr,
                             NET_CONNECT_NONBLOCK);
}

int NetHandler::tcpNonBlockBestEffortBindConnect(char *err,
                                                 char *addr,
                                                 int port,
                                                 char *source_addr) {
    return tcpGenericConnect(err, addr, port, source_addr,
                             NET_CONNECT_NONBLOCK | NET_CONNECT_BE_BINDING);
}

int NetHandler::tcpAccept(char *err,
                          int s,
                          char *ip,
                          size_t iplen,
                          int *port) {
    struct sockaddr sa;
    socklen_t salen = sizeof(sa);

    // accept
    int fd = tcpGenericAccept(err, s, &sa, &salen);
    if(-1 == fd) {
        return -1;
    }

    // analysis the ip and port
    if (AF_INET == sa.sa_family) {
        struct sockaddr_in *sin = (struct sockaddr_in *) &sa;
        if (NULL != ip) {
            inet_ntop(AF_INET, (void *)&sin->sin_addr, ip, iplen);
        }
        if (NULL != port) {
            *port = ntohs(sin->sin_port);
        }
    } else {
        struct sockaddr_in6 *sin = (struct sockaddr_in6 *) &sa;
        if (NULL != ip) {
            inet_ntop(AF_INET6, (void*)&sin->sin6_addr, ip, iplen);
        }
        if (NULL != port) {
            *port = ntohs(sin->sin6_port);
        }
    }

    return fd;
}

int NetHandler::unixAccept(char *err, int s) {
    struct sockaddr sa;
    socklen_t salen = sizeof(sa);
    return tcpGenericAccept(err, s, &sa, &salen);
}

int NetHandler::genericResolve(char *err,
                               char *host,
                               char *ipbuf,
                               size_t ipbuf_len,
                               int flags) {
    struct addrinfo hints, *info;
    int rv;

    memset(&hints, 0, sizeof(hints));
    if (flags & NET_IP_ONLY) hints.ai_flags = AI_NUMERICHOST;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;  /* specify socktype to avoid dups */

    if ((rv = getaddrinfo(host, NULL, &hints, &info)) != 0) {
        setError(err, "%s", gai_strerror(rv));
        return -1;
    }
    if (info->ai_family == AF_INET) {
        struct sockaddr_in *sa = (struct sockaddr_in *)info->ai_addr;
        inet_ntop(AF_INET, &(sa->sin_addr), ipbuf, ipbuf_len);
    } else {
        struct sockaddr_in6 *sa = (struct sockaddr_in6 *)info->ai_addr;
        inet_ntop(AF_INET6, &(sa->sin6_addr), ipbuf, ipbuf_len);
    }

    freeaddrinfo(info);
    return 1;
}

int NetHandler::setReuseAddr(char *err, int fd) {
    int yes = 1;

    if (-1 == setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))) {
        setError(err, "setsockopt SO_REUSEADDR: %s", strerror(errno));
        return -1;
    }
    return 1;
}

int NetHandler::setListen(char *err,
                          int s,
                          struct sockaddr *sa,
                          socklen_t len,
                          int backlog) {
    if (bind(s, sa, len) == -1) {
        setError(err, "bind: %s", strerror(errno));
        close(s);
        return -1;
    }

    if (listen(s, backlog) == -1) {
        setError(err, "listen: %s", strerror(errno));
        close(s);
        return -1;
    }
    return 1;
}

int NetHandler::tcpGenericConnect(char *err,
                                  char *addr,
                                  int port,
                                  char *source_addr,
                                  int flags) {
    int s = -1, rv;
    char portstr[6];  /* strlen("65535") + 1; */
    struct addrinfo hints, *servinfo, *bservinfo, *p, *b;

    snprintf(portstr, sizeof(portstr), "%d", port);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(addr, portstr, &hints, &servinfo)) != 0) {
        setError(err, "%s", gai_strerror(rv));
        return -1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        /**
         * 创建并connect socket, 如果这个servinfo失败了，则尝试下一个
         **/
        if (-1 == (s = socket(p->ai_family, p->ai_socktype, p->ai_protocol))) {
            continue;
        }

        // reuse address
        if (-1 == setReuseAddr(err, s)) {
            goto error;

        }
        // set block
        if (flags & NET_CONNECT_NONBLOCK && -1 == setBlock(err, s, 0)) {
            goto error;
        }

        if (NULL != source_addr) {
            int bound = 0;
            // 获得source地址
            if ((rv = getaddrinfo(
                    source_addr,
                    NULL,
                    &hints,
                    &bservinfo)) != 0) {
                setError(err, "%s", gai_strerror(rv));
                goto error;
            }

            // 绑定地址
            for (b = bservinfo; b != NULL; b = b->ai_next) {
                if (bind(s, b->ai_addr, b->ai_addrlen) != -1) {
                    bound = 1;
                    break;
                }
            }
            freeaddrinfo(bservinfo);
            if (!bound) {
                setError(err, "bind: %s", strerror(errno));
                goto error;
            }
        }
        if (connect(s, p->ai_addr, p->ai_addrlen) == -1) {
            /* If the socket is non-blocking, it is ok for connect() to
             * return an EINPROGRESS error here. */
            if (errno == EINPROGRESS && flags & NET_CONNECT_NONBLOCK) {
                goto end;
            }
            close(s);
            s = -1;
            continue;
        }

        // conect成功
        goto end;
    }
    if (p == NULL) {
        setError(err, "creating socket: %s", strerror(errno));
    }

    error:
    if (s != -1) {
        close(s);
        s = -1;
    }

    end:
    freeaddrinfo(servinfo);

    /**
     * 处理best effort binding: 如果传递了source_addr,
     * 但是创建socket失败，则使用不传递source_addr的方式重试一次
     */
    if (-1 == s && source_addr && (flags & NET_CONNECT_BE_BINDING)) {
        return tcpGenericConnect(err, addr, port, NULL, flags);
    } else {
        return s;
    }
}

int NetHandler::tcpServer(char *err,
                          int port,
                          const char *bindaddr,
                          int backlog) {
    return tcpGenericServer(err, port, bindaddr, AF_INET, backlog);
}

int NetHandler::tcp6Server(char *err,
                           int port,
                           const char *bindaddr,
                           int backlog) {
    return tcpGenericServer(err, port, bindaddr, AF_INET6, backlog);
}

int NetHandler::tcpGenericServer(char *err,
                                 int port,
                                 const char *bindaddr,
                                 int af,
                                 int backlog) {
    int s = -1, rv;
    char _port[6];  /* strlen("65535") */
    struct addrinfo hints, *servinfo, *p;

    snprintf(_port, 6, "%d", port);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = af;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(bindaddr, _port, &hints, &servinfo)) != 0) {
        setError(err, "%s", gai_strerror(rv));
        return -1;
    }
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((s = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
            continue;

        // set ipv6 only
        if (af == AF_INET6 && -1 == setV6Only(err, s)) {
            dealError(s, servinfo);
            return -1;
        }

        // reuse address
        if (-1 == setReuseAddr(err, s)) {
            dealError(s, servinfo);
            return -1;
        }

        // listen to s
        if (-1 == setListen(err, s, p->ai_addr, p->ai_addrlen, backlog)) {
            dealError(s, servinfo);
            return -1;
        }

        // 执行成功
        freeaddrinfo(servinfo);
        return s;
    }

    // 如果所有地址都执行失败，运行错误
    if (NULL == p) {
        setError(err, "unable to bind socket, errno: %d", errno);
        dealError(s, servinfo);
        return -1;
    }
}

int NetHandler::unixServer(char *err,
                           const char *path,
                           mode_t perm,
                           int backlog) {
    int s;
    struct sockaddr_un sa;

    if (-1 == (s = createSocket(err, AF_LOCAL))) {
        return -1;
    }

    memset(&sa, 0, sizeof(sa));
    sa.sun_family = AF_LOCAL;
    strncpy(sa.sun_path, path, sizeof(sa.sun_path) - 1);
    if (-1 == setListen(err, s, (struct sockaddr*)&sa, sizeof(sa), backlog)) {
        return -1;
    }
    if (perm) {
        chmod(sa.sun_path, perm);
    }

    return s;
}

int NetHandler::setBlock(char *err, int fd, int block) {
    int flags;

    if ((flags = fcntl(fd, F_GETFL)) == -1) {
        setError(err, "fcntl(F_GETFL): %s", strerror(errno));
        return -1;
    }

    if (block) {
        flags &= ~O_NONBLOCK;

    } else {
        flags |= O_NONBLOCK;
    }

    if (fcntl(fd, F_SETFL, flags) == -1) {
        setError(err, "fcntl(F_SETFL,O_NONBLOCK): %s", strerror(errno));
        return -1;
    }
    return 1;
}

void NetHandler::setError(char *err, const char *fmt, ...) {
    if (!err) {
        return;
    }

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(err, NET_ERR_LEN, fmt, ap);
    va_end(ap);
}

void NetHandler::dealError(int fd, struct addrinfo *servinfo) {
    if (fd != -1) {
        close(fd);
    }

    freeaddrinfo(servinfo);
}

int NetHandler::tcpGenericAccept(char *err, int s,
                                 struct sockaddr *sa, socklen_t *len) {
    while (1) {
        int fd = accept(s, sa, len);
        if (-1 == fd) {
            if (EINTR == errno) {
                continue;
            } else {
                setError(err, "accept: %s", strerror(errno));
                return -1;
            }
        }

        return fd;
    }
}

int NetHandler::processInputBuffer(const AbstractCoordinator* coordinator,
                                   AbstractFlyClient *flyClient) {
    while (flyClient->getQueryBufSize() > 0) {
        // 第一个字符是'*'代表是整体multibulk串;
        // reqtype=multibulk代表是上次读取已经处理了部分的multibulk
        if ('*' == flyClient->getFirstQueryChar()
            || flyClient->isMultiBulkType()) {
            // 读取失败，直接返回，不做命令处理, inline buffer同理
            if (-1 == processMultiBulkBuffer(coordinator, flyClient)) {
                return -1;
            }
        } else {
            if (-1 == processInlineBuffer(flyClient)) {
                return -1;
            }
        }
    }

    // 处理命令
    coordinator->getFlyServer()->dealWithCommand(flyClient);
    return 1;
}

int NetHandler::writeToClient(const AbstractCoordinator *coordinator,
                              AbstractFlyClient *flyClient,
                              int handlerInstalled) {
    ssize_t onceCount = 0, totalCount = 0;
    int fd = flyClient->getFd();

    // 循环写入
    while (!flyClient->hasNoPending()) {
        if (0 != flyClient->getBufpos()) {
            // 通过网络发送出去
            onceCount = write(flyClient->getFd(),
                              flyClient->getBuf() + flyClient->getSendLen(),
                              flyClient->getBufpos() - flyClient->getSendLen());
            if (onceCount <= 0) {
                break;
            }
            flyClient->addSendLen(onceCount);
            totalCount += onceCount;

            // 固定buf全部发送完
            if (flyClient->bufSendOver()) {
                flyClient->clearBuf();
            }
        } else {
            std::string* reply = flyClient->getReply().front();
            int replyLen = reply->size();
            int sentLen = flyClient->getSendLen();

            onceCount = write(fd, reply->c_str() + sentLen, replyLen - sentLen);
            if (onceCount <= 0) {
                break;
            }
            flyClient->addSendLen(onceCount);
            totalCount += onceCount;

            // 该reply发送完毕
            if (flyClient->getSendLen() == replyLen) {
                flyClient->setSendLen(0);
                flyClient->replyPopFront();
            }
        }

        // 达到最大发送长度
        if (totalCount > NET_MAX_WRITES_PER_EVENT) {
            break;
        }
    }

    // 如果写入过程出错，删除flyClient
    if (onceCount <= 0 && EAGAIN != errno) {
        logHandler->logVerbose("Error writing to client: %s", strerror(errno));
        coordinator->getFlyServer()->freeClientAsync(flyClient);
        close(fd);
        return -1;
    }

    // 统计服务端总共发送的字节数
    coordinator->getFlyServer()->addToStatNetInputBytes(totalCount);

    // 如果全部发送完
    if (flyClient->hasNoPending()) {
        // 删除hanlder
        flyClient->setSendLen(0);
        if (handlerInstalled) {
            coordinator->getEventLoop()->deleteFileEvent(fd, ES_WRITABLE);
        }

        if (flyClient->getFlags() & CLIENT_CLOSE_AFTER_REPLY) {
            coordinator->getFlyServer()->freeClientAsync(flyClient);
            close(fd);
            return -1;
        }
    }

    // 不管是否发送完，都返回1
    return 1;
}

int NetHandler::wait(int fd, int mask, int millseconds) {
    /** 初始化pfd */
    struct pollfd pfd;
    memset(&pfd, 0, sizeof(pfd));
    pfd.fd = fd;
    if (mask & ES_READABLE) {
        pfd.events |= POLLIN;
    }
    if (mask & ES_WRITABLE) {
        pfd.events |= POLLOUT;
    }

    /**
     * 获取设备符状态：
     *      如果返回 = 0，表示没有设备符准备好
     *      如果返回 < 0, 表示出错
     **/
    int retval, retmask;
    if ((retval = poll(&pfd, 1, millseconds)) <= 0) {
        return retval;
    }

    /** 设置设备符可读可写状态mask并返回 */
    if ((pfd.revents & POLLIN) && (mask & ES_READABLE)) {
        retmask |= ES_READABLE;
    }
    if ((pfd.revents & POLLOUT) && (mask & ES_WRITABLE)) {
        retmask |= ES_WRITABLE;
    }
    return retmask;
}

ssize_t NetHandler::syncRead(int fd,
                             char *ptr,
                             ssize_t size,
                             uint64_t timeout) {
    ssize_t readCount = 0;
    uint64_t start = miscTool->mstime(), span = 0;
    while ((span = miscTool->mstime() - start) < timeout) {
        /** read once */
        int readRes;
        if ((readRes = read(fd, ptr + readCount, size - readCount) <= 0)) {
            if (-1 == readRes && EAGAIN == errno) {
                continue;
            }
            return -1;
        }

        /** weather read all? */
        readCount += readRes;
        if (readCount == size) {
            break;
        }

        /** wait fd readable */
        this->wait(fd, ES_READABLE, timeout - span);
    }

    return readCount;
}

ssize_t NetHandler::syncWrite(int fd,
                              char *ptr,
                              ssize_t size,
                              uint64_t timeout) {
    uint64_t start = miscTool->mstime(), span = 0;
    ssize_t totalWrite = 0;

    while ((span = miscTool->mstime() - start) < timeout) {
        int writeRes = 0;
        if ((writeRes = write(fd, ptr + totalWrite, size - totalWrite)) <= 0) {
            /** if res is -1 and errno means read again, then continue */
            if (-1 == writeRes && EAGAIN == errno) {
                continue;
            }

            return -1;
        }

        /** if write all done, break and return */
        totalWrite += writeRes;
        if (writeRes == size) {
            break;
        }

        /** wait fd writable */
        this->wait(fd, ES_WRITABLE, timeout - span);
    }

    return totalWrite;
}

int NetHandler::processInlineBuffer(AbstractFlyClient *flyClient) {
    size_t pos = flyClient->getQueryBuf().find("\r\n");
    if (pos == flyClient->getQueryBuf().npos) {     // 没有找到
        if (flyClient->getQueryBufSize() > PROTO_INLINE_MAX_SIZE) {
            addReplyError(flyClient,
                          "Protocol error: too big mbulk count string");
            setProtocolError("too big mbulk count string", flyClient, 0);
        }
        return -1;
    }

    std::vector<std::string> words;
    std::string subStr = flyClient->getQueryBuf().substr(0, pos);
    miscTool->spiltString(subStr, " ", words);
    if (0 == words.size()) {
        return -1;
    }

    // 如果参数列表不为空，先释放空间再重新分配
    if (NULL != flyClient->getArgv()) {
        flyClient->freeArgv();
    }
    flyClient->allocArgv(words.size());

    // 设置参数列表
    extern AbstractCoordinator *coordinator;
    for (auto item : words) {
        coordinator->getFlyObjStringFactory()->getObject(new std::string(item));
    }

    // 裁剪输入缓冲区
    flyClient->trimQueryBuf(pos + 2, -1);
    return 1;
}

/**
 * 用于处理RESP协议的请求。将client的query buffer拆解到client的argv中。
 * 如果成功，返回0；
 * 否则返回-1，有如下两种情况：
 *  1.当前的query buffer中还不足以解析出一个完整的command，需要等待下次读取完
 *  2.当前协议解析失败，此时需要中断和客户端的连接
 */
int NetHandler::processMultiBulkBuffer(const AbstractCoordinator* coordinator,
                                       AbstractFlyClient *flyClient) {
    size_t pos = 0;
    flyClient->setReqType(PROTO_REQ_MULTIBULK);

    // multi bulk等于0，代表是新的multibulk字符串，
    // 否则就是未读完的部分(未读完的部分不用重新读取multi bulk len)
    if (0 == flyClient->getMultiBulkLen()) {
        // 如果获取multi bulk len失败，返回-1
        if (-1 == analyseMultiBulkLen(flyClient, pos)) {
            return -1;
        }
    }

    // 解析multi bulk
    if (-1 == analyseMultiBulk(coordinator, flyClient, pos)) {
        return -1;
    }

    // 所有bulk读取完，重置如下字段，表示这次multi bulk处理完
    flyClient->setMultiBulkLen(0);
    flyClient->setReqType(0);
    return 1;
}

int NetHandler::analyseMultiBulkLen(AbstractFlyClient *flyClient, size_t &pos) {
    pos = flyClient->getQueryBuf().find("\r\n");
    if (pos == flyClient->getQueryBuf().npos) {     // 没有找到
        if (flyClient->getQueryBufSize() > PROTO_INLINE_MAX_SIZE) {
            addReplyError(flyClient,
                          "Protocol error: too big mbulk count string");
            setProtocolError("too big mbulk count string", flyClient, 0);
        }
        return -1;
    }

    int64_t multiBulkLen = 0;
    std::string subStr = flyClient->getQueryBuf().substr(1, pos - 1);
    int res = miscTool->string2int64(subStr, multiBulkLen);

    // 如果获取multi bulk length失败，或者其太长，协议error
    if (0 == res || multiBulkLen > PROTO_REQ_MULTIBULK_MAX_LEN) {
        addReplyError(flyClient, "Protocol error: invalid multibulk length");
        setProtocolError("invalid mbulk count", flyClient, pos);
        return -1;
    }

    pos += 2;
    /**
     * 如果multi bulk len < 0, 表示null, 该multi bulk命令读取完毕
     * 此时由于client->multiBulkLen = 0, 不会执行外围函数的后续bulk解析
     */
    if (multiBulkLen < 0) {
        flyClient->trimQueryBuf(pos, -1);
        return 1;
    }

    // 设置multi bulk len
    flyClient->setMultiBulkLen(multiBulkLen);

    // 如果参数列表不为空，先释放空间再重新分配
    if (NULL != flyClient->getArgv()) {
        flyClient->freeArgv();
    }
    flyClient->allocArgv(multiBulkLen);

    // 截取输入字符串
    flyClient->trimQueryBuf(pos, -1);
    return 1;
}

int NetHandler::analyseMultiBulk(const AbstractCoordinator* coordinator,
                                 AbstractFlyClient *flyClient,
                                 size_t &pos) {
    int64_t multiBulkLen = flyClient->getMultiBulkLen();
    for (int i = flyClient->getArgc(); i < multiBulkLen; i++) {
        if (-1 == analyseBulk(coordinator, flyClient)) {
            return -1;
        }
    }
}

int NetHandler::analyseBulk(const AbstractCoordinator* coordinator,
                            AbstractFlyClient *flyClient) {
    // 如果字符串为空，说明需要通过下次来读取，直接返回(但并不是协议错误)
    if (0 == flyClient->getQueryBuf().size()) {
        return -1;
    }

    size_t pos = 0;
    if ('$' != flyClient->getQueryBuf()[pos]) {
        addReplyErrorFormat(flyClient, "Protocol error: expected '$', got '%c'",
                            flyClient->getQueryBuf()[pos]);
        setProtocolError("expected $ but got something else", flyClient, pos);
        return -1;
    }

    // 获取到"\r\n"的位置
    size_t begin = pos + 1;
    pos = flyClient->getQueryBuf().find("\r\n", begin);
    if (pos == flyClient->getQueryBuf().npos) {     // 没有找到
        if (flyClient->getQueryBufSize() > PROTO_INLINE_MAX_SIZE) {
            addReplyError(flyClient,
                          "Protocol error: too big bulk count string");
            setProtocolError("too big bulk count string", flyClient, 0);
        }
        return -1;
    }

    // 从client->querybuf里截取该bulk
    int64_t bulkLen = 0;
    std::string subStr = flyClient->getQueryBuf().substr(begin, pos - begin);
    int res = miscTool->string2int64(subStr, bulkLen);
    if (0 == res || bulkLen < 0 || bulkLen > PROTO_REQ_BULK_MAX_LEN) {
        addReplyError(flyClient, "Protocol error: invalid bulk length");
        setProtocolError("invalid bulk length", flyClient, pos);
    }
    begin = pos + 2;

    // 如果读取不全（找不到\r\n），返回-1，等待下次读取
    pos = flyClient->getQueryBuf().find("\r\n", begin);
    if (pos == flyClient->getQueryBuf().npos) {
        return -1;
    }

    // 如果读取全了，并且长度不对，说明是协议问题
    if (pos - begin != bulkLen) {
        addReplyError(flyClient, "Protocol error: not enough bulk space");
        setProtocolError("not enough bulk space", flyClient, pos);
        return -1;
    }

    // 设置flyClient argv参数
    flyClient->addArgv(coordinator->getFlyObjStringFactory()->getObject(
            new std::string(flyClient->getQueryBuf().substr(begin, pos - begin))));

    // 截取此次读取
    flyClient->trimQueryBuf(pos + 2, -1);
    return 1;
}

int NetHandler::setProtocolError(
        char *err,
        AbstractFlyClient *flyClient,
        size_t pos) {
    // 打印log
    char buf[256];
    snprintf(buf, sizeof(buf), "Query buffer during protocol error: '%s'",
             flyClient->getQueryBuf().c_str());
    logHandler->logVerbose("Protocol error (%s) from client: %ld. %s",
                           err, flyClient->getId(), buf);

    // 设置回复后关闭
    flyClient->addFlag(CLIENT_CLOSE_AFTER_REPLY);
    // 截断query buf
    flyClient->trimQueryBuf(pos + 2, -1);
}

void NetHandler::addReplyErrorFormat(AbstractFlyClient *flyClient,
                                     const char *fmt, ...) {
    va_list ap;
    char msg[1024];
    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);

    int len = strlen(msg);
    // 保证msg中没有换行符, 使msg在一行内
    for (int i = 0; i < len; i++) {
        if ('\r' == msg[i] || '\n' == msg[i]) {
            msg[i] = ' ';
        }
    }

    addReplyError(flyClient, msg);
}

int NetHandler::addReplyError(AbstractFlyClient *flyClient, const char *err) {
    flyClient->addReply("-ERR ", 5);
    flyClient->addReply(err, strlen(err));
    flyClient->addReply("\r\n", 2);
}

void acceptTcpHandler(const AbstractCoordinator *coordinator,
                      int fd,
                      void *clientdata,
                      int mask) {
    AbstractFlyServer *flyServer = coordinator->getFlyServer();
    AbstractNetHandler *netHandler = coordinator->getNetHandler();

    int cfd, cport;
    char cip[NET_IP_STR_LEN];

    for (int i = 0; i < MAX_ACCEPTS_PER_CALL; i++) {
        cfd = netHandler->tcpAccept(NULL, fd, cip, sizeof(cip), &cport);
        if (-1 == cfd) {
            return;
        }

        AbstractFlyClient* flyClient = flyServer->createClient(cfd);
        if (NULL == flyClient) {
            std::cout<< "error to create fly client" << std::endl;
            close(cfd);
        }
    }
}

void readQueryFromClient(const AbstractCoordinator *coordinator,
                         int fd,
                         void *clientdata,
                         int mask) {
    AbstractFlyServer *flyServer = coordinator->getFlyServer();
    AbstractNetHandler *netHandler = coordinator->getNetHandler();
    AbstractFlyClient *flyClient =
            reinterpret_cast<AbstractFlyClient *>(clientdata);

    char buf[PROTO_IOBUF_LEN];
    int readCnt = read(fd, buf, sizeof(buf));
    // 读取失败, 如果错误码是EAGAIN说明本次读取没数据, 则直接返回
    // 否则需要删除client
    if (-1 == readCnt) {
        if (EAGAIN == errno) {
            return;
        } else {                                // 连接异常
            flyServer->deleteClient(fd);
            close(fd);
            return;
        }
    } else if (0 == readCnt) {                  // 关闭连接
        flyServer->deleteClient(fd);
        close(fd);
        return;
    }
    flyClient->addToQueryBuf(buf);
    flyClient->setLastInteractionTime(flyServer->getNowt());

    // 统计flyServer接收到的byte数量
    flyServer->addToStatNetInputBytes(strlen(buf));
    if (flyClient->getQueryBufSize() > flyServer->getClientMaxQuerybufLen()) {
        flyServer->deleteClient(fd);
        close(fd);
        std::cout << "Closing client that reached max query buffer length"
                     << std::endl;
        return;
    }

    netHandler->processInputBuffer(coordinator, flyClient);
}

void sendReplyToClient(const AbstractCoordinator *coordinator,
                       int fd,
                       void *clientdata,
                       int mask) {
    AbstractFlyClient *flyClient =
            reinterpret_cast<AbstractFlyClient *>(clientdata);
    coordinator->getNetHandler()->writeToClient(coordinator, flyClient, 1);
}

