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
    this->logfile = strdup(CONFIG_DEFAULT_LOGFILE.c_str());
    this->verbosity = CONFIG_DEFAULT_VERBOSITY;
    this->syslogEnabled = CONFIG_DEFAULT_SYSLOG_ENABLED;
    this->syslogIdent = strdup(CONFIG_DEFAULT_SYSLOG_IDENT.c_str());

    // fdb相关
    this->fdbFile = strdup(CONFIG_DEFAULT_FDB_FILENAME.c_str());

    // aof相关
    this->aofFile = strdup(CONFIG_DEFAULT_AOF_FILENAME.c_str());
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

mode_t ConfigCache::getUnixsocketperm() const {
    return this->unixsocketperm;
}

void ConfigCache::setUnixsocketperm(mode_t unixsocketperm) {
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

char *ConfigCache::getAofFile() const {
    return this->aofFile;
}

void ConfigCache::setAofFile(char *aofFile) {
    this->aofFile = aofFile;
}

bool ConfigCache::isAofUseFdbPreamble() const {
    return this->aofUseFdbPreamble;
}

void ConfigCache::setAofUseFdbPreamble(bool aofUseFdbPreamble) {
    this->aofUseFdbPreamble = aofUseFdbPreamble;
}

int ConfigCache::getAofFsync() const {
    return this->aofFsync;
}

void ConfigCache::setAofFsync(int aofFsync) {
    this->aofFsync = aofFsync;
}

bool ConfigCache::isAofRewriteIncrementalFsync() const {
    return this->aofRewriteIncrementalFsync;
}

void ConfigCache::setAofRewriteIncrementalFsync(
        bool aofRewriteIncrementalFsync) {
    this->aofRewriteIncrementalFsync = aofRewriteIncrementalFsync;
}

void ConfigCache::setBindAddrs(const std::vector<std::string> &bindAddrs) {
    this->bindAddrs = bindAddrs;
}

bool ConfigCache::isAofNoFsyncOnRewrite() const {
    return this->aofNoFsyncOnRewrite;
}

void ConfigCache::setAofNoFsyncOnRewrite(bool aofNoFsyncOnRewrite) {
    this->aofNoFsyncOnRewrite = aofNoFsyncOnRewrite;
}

int ConfigCache::getAofRewritePerc() const {
    return this->aofRewritePerc;
}

void ConfigCache::setAofRewritePerc(int aofRewritePerc) {
    this->aofRewritePerc = aofRewritePerc;
}

off_t ConfigCache::getAofRewriteMinSize() const {
    return this->aofRewriteMinSize;
}

void ConfigCache::setAofRewriteMinSize(off_t aofRewriteMinSize) {
    this->aofRewriteMinSize = aofRewriteMinSize;
}

bool ConfigCache::isAofLoadTruncated() const {
    return this->aofLoadTruncated;
}

void ConfigCache::setAofLoadTruncated(bool aofLoadTruncated) {
    this->aofLoadTruncated = aofLoadTruncated;
}
