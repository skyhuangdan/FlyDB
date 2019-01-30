//
// Created by 赵立伟 on 2018/11/29.
//

#ifndef FLYDB_AOFHANDLER_H
#define FLYDB_AOFHANDLER_H

#include <string>
#include "interface/AbstractAOFHandler.h"
#include "../coordinator/interface/AbstractCoordinator.h"
#include "../io/FileFio.h"
#include "../dataStructure/intset/IntSet.h"

template<class T>
class SkipList;

template<class KEY, class VAL>
class Dict;

class AOFHandler : public AbstractAOFHandler {
public:
    AOFHandler();
    int start();
    int rewriteBackground();
    int rewriteAppendOnlyFile();
    void backgroundSaveDone(int exitCode, int bySignal);

    pid_t getChildPid() const;
    void setChildPid(pid_t childPid);
    bool haveChildPid() const;
    bool IsStateOn() const;
    void setState(AOFState aofState);
    bool isScheduled() const;
    void setScheduled(bool scheduled);

    void setCoordinator(AbstractCoordinator *coordinator);
    void setFileName(char *fileName);
    void setUseFdbPreamble(bool useFdbPreamble);
    void setFsyncStragy(int stragy);
    void setRewriteIncrementalFsync(bool rewriteIncrementalFsync);
    int saveKeyValuePair(Fio *fio,
                         std::string key,
                         std::shared_ptr<FlyObj> val,
                         int64_t expireTime);

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
    static int dbScan(void *priv,
                       std::string key,
                       std::shared_ptr<FlyObj> val);
    static void skipListSaveProc(void *priv, const std::string &obj);
    static int dictSaveScan(void *priv, std::string key, std::string val);
    int rewriteAppendOnlyFileDiff(char *tmpfile, FileFio* fio);
    int rewriteAppendOnlyFileFio(Fio *fio);
    int readDiffFromParent();
    int rewriteList(Fio *fio,
                    std::string key,
                    std::list<std::string> *val);
    int rewriteSkipList(Fio *fio,
                        std::string key,
                        SkipList<std::string> *skipList);
    int rewriteHashTable(Fio *fio,
                         std::string key,
                         Dict<std::string, std::string> *dict);
    int rewriteIntSet(Fio *fio, std::string key, IntSet *intset);

    AbstractCoordinator *coordinator;
    char *fileName;
    AOFState state;
    pid_t childPid = -1;
    std::string childDiff;
    time_t lastFsync;
    int fd = -1;
    bool scheduled = false;
    bool useFdbPreamble = CONFIG_DEFAULT_AOF_USE_FDB_PREAMBLE;
    int fsyncStragy = CONFIG_DEFAULT_AOF_FSYNC;
    bool rewriteIncrementalFsync = CONFIG_DEFAULT_AOF_REWRITE_INCREMENTAL_FSYNC;
    bool stopSendingDiff = false;

};


#endif //FLYDB_AOFHANDLER_H
