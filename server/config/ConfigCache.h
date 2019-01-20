//
// Created by 赵立伟 on 2018/12/5.
//

#ifndef FLYDB_FILECONFIG_H
#define FLYDB_FILECONFIG_H

#include <string>
#include <vector>
#include "../aof/AOFDef.h"

struct ConfigCache {
public:

    ConfigCache();

    const std::vector<std::string> &getBindAddrs() const;

    void addBindAddr(const std::string &bindAddr);

    const char *getUnixsocket() const;

    void setUnixsocket(const char *unixsocket);

    mode_t getUnixsocketperm() const;

    void setUnixsocketperm(mode_t unixsocketperm);

    int getTcpKeepAlive() const;

    void setTcpKeepAlive(int tcpKeepAlive);

    int getVerbosity() const;

    void setVerbosity(int verbosity);

    char *getLogfile() const;

    void setLogfile(char *logfile);

    int getSyslogEnabled() const;

    void setSyslogEnabled(int syslogEnabled);

    char *getSyslogIdent() const;

    void setSyslogIdent(char *syslogIdent);

    char *getFdbFile() const;

    void setFdbFile(char *fdbFile);

    char *getAofFile() const;

    void setAofFile(char *aofFile);

    AOFState getAofState() const;

    void setAofState(AOFState aofState);

    int getPort() const;

    void setPort(int port);

    int getSyslogFacility() const;

    void setSyslogFacility(int syslogFacility);

private:
    /**
     * 网络相关
     * */
    std::vector<std::string> bindAddrs;             // 绑定地址
    const char *unixsocket;                         // UNIX socket path
    mode_t unixsocketperm;                          // UNIX socket permission
    int tcpKeepAlive;
    int port;                                       // tcp listening port

    /**
     * log相关
     */
    int verbosity;                                  // log level in log file
    char *logfile;                                  // log file
    int syslogEnabled;                              // 是否开启log
    char *syslogIdent;                              // log标记
    int syslogFacility;

    /**
     * 持久化
     */
    char *fdbFile;                                  // fdb持久化文件名
    char *aofFile;                                  // aof持久化文件名
    AOFState aofState;
};

#endif //FLYDB_FILECONFIG_H
