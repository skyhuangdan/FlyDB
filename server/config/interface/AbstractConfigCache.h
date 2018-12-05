//
// Created by 赵立伟 on 2018/12/5.
//

#ifndef FLYDB_ICONFIG_H
#define FLYDB_ICONFIG_H

#include "../../aof/AOFDef.h"

class AbstractConfigCache {
public:
    virtual const std::vector<std::string> &getBindAddrs() const = 0;

    virtual void addBindAddr(const std::string &bindAddr) = 0;

    virtual const char *getUnixsocket() const = 0;

    virtual void setUnixsocket(const char *unixsocket) = 0;

    virtual mode_t getUnixsocketperm() const = 0;

    virtual void setUnixsocketperm(mode_t unixsocketperm) = 0;

    virtual int getTcpKeepAlive() const = 0;

    virtual void setTcpKeepAlive(int tcpKeepAlive) = 0;

    virtual int getVerbosity() const = 0;

    virtual void setVerbosity(int verbosity) = 0;

    virtual char *getLogfile() const = 0;

    virtual void setLogfile(char *logfile) = 0;

    virtual int getSyslogEnabled() const = 0;

    virtual void setSyslogEnabled(int syslogEnabled) = 0;

    virtual char *getSyslogIdent() const = 0;

    virtual void setSyslogIdent(char *syslogIdent) = 0;

    virtual char *getFdbFile() const = 0;

    virtual void setFdbFile(char *fdbFile) = 0;

    virtual AOFState getAofState() const = 0;

    virtual void setAofState(AOFState aofState) = 0;

    virtual int getPort() const = 0;

    virtual void setPort(int port) = 0;

    virtual int getSyslogFacility() const = 0;

    virtual void setSyslogFacility(int syslogFacility) = 0;

};

#endif //FLYDB_ICONFIG_H
