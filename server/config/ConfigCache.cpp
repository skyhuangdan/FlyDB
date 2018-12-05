//
// Created by 赵立伟 on 2018/12/5.
//

#include <vector>
#include "ConfigCache.h"
#include "../aof/AOFDef.h"
#include "../../def.h"

ConfigCache::ConfigCache() {
    this->port = CONFIG_DEFAULT_SERVER_PORT;
    // unix domain socket
    this->unixsocket = NULL;
    this->unixsocketperm = CONFIG_DEFAULT_UNIX_SOCKET_PERM;

    // keep alive
    this->tcpKeepAlive = CONFIG_DEFAULT_TCP_KEEPALIVE;

    // log相关
    this->verbosity = CONFIG_DEFAULT_VERBOSITY;
    this->syslogEnabled = CONFIG_DEFAULT_SYSLOG_ENABLED;
    this->logfile = strdup(CONFIG_DEFAULT_LOGFILE.c_str());
    this->syslogIdent = strdup(CONFIG_DEFAULT_SYSLOG_IDENT.c_str());

    // fdb相关
    this->fdbFile = strdup(CONFIG_DEFAULT_FDB_FILENAME.c_str());
}

const std::vector <std::string> &ConfigCache::getBindAddrs() const {
    return this->bindAddrs;
}

void ConfigCache::addBindAddr(const std::string &bindAddr) {
    this->bindAddrs.push_back(bindAddr);
}

const char *ConfigCache::getUnixsocket() const {
    return this->unixsocket;
}

void ConfigCache::setUnixsocket(const char *unixsocket) {
    this->unixsocket = unixsocket;
}

unsigned short ConfigCache::getUnixsocketperm() const {
    return this->unixsocketperm;
}

void ConfigCache::setUnixsocketperm(unsigned short unixsocketperm) {
    this->unixsocketperm = unixsocketperm;
}

int ConfigCache::getTcpKeepAlive() const {
    return this->tcpKeepAlive;
}

void ConfigCache::setTcpKeepAlive(int tcpKeepAlive) {
    this->tcpKeepAlive = tcpKeepAlive;
}

int ConfigCache::getVerbosity() const {
    return this->verbosity;
}

void ConfigCache::setVerbosity(int verbosity) {
    this->verbosity = verbosity;
}

char *ConfigCache::getLogfile() const {
    return this->logfile;
}

void ConfigCache::setLogfile(char *logfile) {
    this->logfile = logfile;
}

int ConfigCache::getSyslogEnabled() const {
    return this->syslogEnabled;
}

void ConfigCache::setSyslogEnabled(int syslogEnabled) {
    this->syslogEnabled = syslogEnabled;
}

char *ConfigCache::getSyslogIdent() const {
    return this->syslogIdent;
}

void ConfigCache::setSyslogIdent(char *syslogIdent) {
    this->syslogIdent = syslogIdent;
}

char *ConfigCache::getFdbFile() const {
    return this->fdbFile;
}

void ConfigCache::setFdbFile(char *fdbFile) {
    this->fdbFile = fdbFile;
}

AOFState ConfigCache::getAofState() const {
    return this->aofState;
}

void ConfigCache::setAofState(AOFState aofState) {
    this->aofState = aofState;
}

int ConfigCache::getPort() const {
    return this->port;
}

void ConfigCache::setPort(int port) {
    this->port = port;
}

int ConfigCache::getSyslogFacility() const {
    return this->syslogFacility;
}

void ConfigCache::setSyslogFacility(int syslogFacility) {
    this->syslogFacility = syslogFacility;
}

