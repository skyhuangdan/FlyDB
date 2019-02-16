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
    int stop();
    int rewriteBackground();
    int rewriteAppendOnlyFile();
    void backgroundSaveDone(int exitCode, int bySignal);

    pid_t getChildPid() const;
    void setChildPid(pid_t childPid);
    bool haveChildPid() const;
    bool IsStateOn() const;
    bool IsStateOff() const;
    void setState(AOFState aofState);
    bool isScheduled() const;
    void setScheduled(bool scheduled);

    void setCoordinator(AbstractCoordinator *coordinator);
    void setFileName(char *fileName);
    void setUseFdbPreamble(bool useFdbPreamble);
    void setFsyncStragy(int stragy);
    void setRewriteIncrementalFsync(bool rewriteIncrementalFsync);
    void setRewritePerc(int rewritePerc);
    void setRewriteMinSize(off_t rewriteMinSize);
    void setNoFsyncOnRewrite(bool noFsyncOnRewrite);
    void setLoadTruncated(bool loadTruncated);
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

        Builder& rewritePerc(int perc) {
            this->handler->setRewritePerc(perc);
            return *this;
        }

        Builder& noFsyncOnRewrite(bool nofyncOnRewrite) {
            this->handler->setNoFsyncOnRewrite(nofyncOnRewrite);
            return *this;
        }

        Builder& rewriteMinSize(off_t rewriteMinSize) {
            this->handler->setRewriteMinSize(rewriteMinSize);
            return *this;
        }

        Builder& loadTruncated(bool loadTruncated) {
            this->handler->setLoadTruncated(loadTruncated);
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
    static void childPipeReadable(const AbstractCoordinator *coordinator,
                                  int fd,
                                  void *clientdata,
                                  int mask);
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
    void removeTempFile(pid_t childpid);

    static bool stopSendingDiff;

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
    /** Rewrite AOF if % growth is > M */
    int rewritePerc = AOF_REWRITE_PERC;
    /** AOf file is at least N bytes */
    off_t rewriteMinSize = AOF_REWRITE_MIN_SIZE;
    /** Don`t fsync when the rewrite iis in progress */
    bool noFsyncOnRewrite = CONFIG_DEFAULT_AOF_NO_FSYNC_ON_REWRITE;
    /** Don't stop on unexpected AOF EOF. */
    bool loadTruncated = CONFIG_DEFAULT_AOF_LOAD_TRUNCATED;
};


#endif //FLYDB_AOFHANDLER_H
