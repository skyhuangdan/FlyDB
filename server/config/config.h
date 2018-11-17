//
// Created by 赵立伟 on 2018/10/4.
//

#ifndef FLYDB_CONFIG_H
#define FLYDB_CONFIG_H

#include <string>
#include <syslog.h>

#ifndef BYTE_ORDER
#if (BSD >= 199103)
# include <machine/endian.h>
#else
#if defined(linux) || defined(__linux__)
# include <endian.h>
#else
#define LITTLE_ENDIAN 1234	    /* least-significant byte first (vax, pc) */
#define BIG_ENDIAN 4321	        /* most-significant byte first (IBM, net) */

#if defined(__i386__) || defined(__x86_64__) || defined(__amd64__) || \
   defined(vax) || defined(ns32000) || defined(sun386) || \
   defined(MIPSEL) || defined(_MIPSEL) || defined(BIT_ZERO_ON_RIGHT) || \
   defined(__alpha__) || defined(__alpha)
#define BYTE_ORDER    LITTLE_ENDIAN
#endif

#if defined(sel) || defined(pyr) || defined(mc68000) || defined(sparc) || \
    defined(is68k) || defined(tahoe) || defined(ibm032) || defined(ibm370) || \
    defined(MIPSEB) || defined(_MIPSEB) || defined(_IBMR2) || defined(DGUX) ||\
    defined(apollo) || defined(__convex__) || defined(_CRAY) || \
    defined(__hppa) || defined(__hp9000) || \
    defined(__hp9000s300) || defined(__hp9000s700) || \
    defined (BIT_ZERO_ON_LEFT) || defined(m68k) || defined(__sparc)
#define BYTE_ORDER	BIG_ENDIAN
#endif
#endif /* linux */
#endif /* BSD */
#endif /* BYTE_ORDER */

struct configMap {
    const char *name;
    const int value;
};

const int CONFIG_CRON_HZ = 1;
const int CONFIG_DEFAULT_SERVER_PORT = 6379;                    // TCP port
const int CONFIG_DEFAULT_MAX_CLIENTS = 10000;                   // 最多同时连接client数量
const int CONFIG_MIN_RESERVED_FDS = 32;                         // 保留文件数量
const int CONFIG_FDSET_INCR = (CONFIG_MIN_RESERVED_FDS + 96);   // eventloop可以处理的文件事件数量
const int CONFIG_BINDADDR_MAX = 16;
const int CONFIG_MAX_LINE = 1024;
const int CONFIG_DEFAULT_UNIX_SOCKET_PERM = 0;
const int CONFIG_DEFAULT_TCP_KEEPALIVE = 300;

// log相关
const int CONFIG_DEFAULT_SYSLOG_ENABLED = 0;
const std::string CONFIG_DEFAULT_LOGFILE = "";
const std::string CONFIG_DEFAULT_SYSLOG_IDENT = "redis";
enum logLevel {
    LL_DEBUG,
    LL_VERBOSE,
    LL_NOTICE,
    LL_WARNING,
    LL_RAW = (1 << 10)
};
const int CONFIG_DEFAULT_VERBOSITY = LL_NOTICE;

#endif //FLYDB_CONFIG_H
