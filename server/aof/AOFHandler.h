//
// Created by 赵立伟 on 2018/11/29.
//

#ifndef FLYDB_AOFHANDLER_H
#define FLYDB_AOFHANDLER_H

#include "interface/AbstractAOFHandler.h"
#include "../coordinator/interface/AbstractCoordinator.h"

class AOFHandler : public AbstractAOFHandler {
public:
    AOFHandler();
    int start();
    int rewriteBackground();
    int rewriteAppendOnlyFile();

    pid_t getChildPid() const;
    void setChildPid(pid_t childPid);
    bool haveChildPid() const;
    bool IsStateOn() const;
    void setState(AOFState aofState);

    void setCoordinator(AbstractCoordinator *coordinator);
    void setFileName(char *fileName);
    void setUseFdbPreamble(bool useFdbPreamble);
    void setFsyncStragy(int stragy);
    void setRewriteIncrementalFsync(bool rewriteIncrementalFsync);

    class Builder {
    public:
        Builder() {
            this->handler = new AOFHandler();
        }

        Builder& coordinator(AbstractCoordinator *coord) {
            this->handler->setCoordinator(coord);
            return *this;
        }

        Builder& fileName(char *fileName) {
            this->handler->setFileName(fileName);
            return *this;
        }

        Builder& state(AOFState aofState) {
            this->handler->setState(aofState);
            return *this;
        }

        Builder& useFdbPreamble(bool useFdbPreamble) {
            this->handler->setUseFdbPreamble(useFdbPreamble);
            return *this;
        }

        Builder& fsyncStragy(int stragy) {
            this->handler->setFsyncStragy(stragy);
            return *this;
        }

        Builder& rewriteIncrementalFsync(bool rewriteIncrementalFsync) {
            this->handler->setRewriteIncrementalFsync(rewriteIncrementalFsync);
            return *this;
        }

        AOFHandler* build() {
            return this->handler;
        }

    private:
        AOFHandler *handler;
    };

private:
    int rewriteAppendOnlyFileFio(Fio *fio);

    AbstractCoordinator *coordinator;
    char *fileName;
    AOFState state;
    pid_t childPid = -1;
    std::string aofBuf;
    time_t lastFsync;
    int fd = -1;
    bool scheduled = false;
    bool useFdbPreamble = CONFIG_DEFAULT_AOF_USE_FDB_PREAMBLE;
    int fsyncStragy = CONFIG_DEFAULT_AOF_FSYNC;
    bool rewriteIncrementalFsync = CONFIG_DEFAULT_AOF_REWRITE_INCREMENTAL_FSYNC;

};


#endif //FLYDB_AOFHANDLER_H
