//
// Created by 赵立伟 on 2018/11/29.
//

#ifndef FLYDB_AOFHANDLER_H
#define FLYDB_AOFHANDLER_H

#include <list>
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
    void flush(bool force);
    int load();
    void addToBuf(std::string addBuf);
    void rewriteBufferAppend(std::string buf);
    int rewriteBackground();
    void backgroundSaveDone(int exitCode, int bySignal);
    void feedAppendOnlyFile(int dbid, std::shared_ptr<FlyObj> *argv, int argc);

    int getFd() const;
    pid_t getChildPid() const;
    void setChildPid(pid_t childPid);
    bool haveChildPid() const;
    bool IsStateOn() const;
    bool IsStateOff() const;
    bool IsStateWaitRewrite() const;
    void setState(AOFState aofState);
    bool isScheduled() const;
    void setScheduled(bool scheduled);
    bool sizeMeetRewriteCondition();
    int saveKeyValuePair(Fio *fio,
                         std::string key,
                         std::shared_ptr<FlyObj> val,
                         int64_t expireTime);
    bool flushPostponed() const;
    bool lastWriteHasError() const;
    void removeTempFile();
    RewriteBufBlock* getFrontRewriteBufBlock() const;
    void popFrontRewriteBufBlock();
    int getSelectedDB() const;
    void setSelectedDB(int selectedDB);

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

        Builder& rewritePerc(uint8_t perc) {
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
    int rewriteAppendOnlyFile();
    static int dbScan(void *priv,
                       std::string key,
                       std::shared_ptr<FlyObj> val);
    static void skipListSaveProc(void *priv, const std::string &obj);
    static int dictSaveScan(void *priv, std::string key, std::string val);
    static void childPipeReadable(const AbstractCoordinator *coordinator,
                                  int fd,
                                  void *clientdata,
                                  int mask);
    static void childWriteDiffData(const AbstractCoordinator *coorinator,
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
    void backgroundFsync();
    void doRealWrite();
    void doRealFsync(bool syncInProgress);
    void terminateWithSuccess();
    void updateCurrentSize();
    void rewriteCleanup();
    int loadRemaindingAOF(FILE *fp);
    std::string catAppendOnlyGenericCommand(std::shared_ptr<FlyObj> *argv,
                                            int argc);

    /** set函数集合，给Builder使用 */
    void setCoordinator(AbstractCoordinator *coordinator);
    void setFileName(char *fileName);
    void setUseFdbPreamble(bool useFdbPreamble);
    void setFsyncStragy(int stragy);
    void setRewriteIncrementalFsync(bool rewriteIncrementalFsync);
    void setRewritePerc(uint8_t rewritePerc);
    void setRewriteMinSize(off_t rewriteMinSize);
    void setNoFsyncOnRewrite(bool noFsyncOnRewrite);
    void setLoadTruncated(bool loadTruncated);
    /** 获取当前rewrite buf的所有block.used总大小 */
    uint64_t getRewriteBufSize();
    /** 清空rewrite buf */
    void clearRewriteBuffer();
    void clearFileEvent();
    ssize_t rewriteBufferWriteToFile(int fd);

    static bool stopSendingDiff;

    AbstractCoordinator *coordinator;
    char *fileName;
    AOFState state;
    pid_t childPid = -1;
    std::string childDiff;
    int fd = -1;
    bool scheduled = false;
    bool useFdbPreamble = CONFIG_DEFAULT_AOF_USE_FDB_PREAMBLE;
    int fsyncStragy = CONFIG_DEFAULT_AOF_FSYNC;
    bool rewriteIncrementalFsync = CONFIG_DEFAULT_AOF_REWRITE_INCREMENTAL_FSYNC;
    /** Rewrite AOF if % growth is > M */
    uint8_t rewritePerc = AOF_REWRITE_PERC;
    /** AOF file is at least N bytes */
    off_t rewriteMinSize = AOF_REWRITE_MIN_SIZE;
    /** AOF size on latest startup or rewrite */
    off_t rewriteBaseSize = 0;
    off_t currentSize = 0;
    /** Don`t fsync when the rewrite iis in progress */
    bool noFsyncOnRewrite = CONFIG_DEFAULT_AOF_NO_FSYNC_ON_REWRITE;
    /** Don't stop on unexpected AOF EOF. */
    bool loadTruncated = CONFIG_DEFAULT_AOF_LOAD_TRUNCATED;
    /** UNIX time of postponed AOF flush */
    time_t flushPostponedStart = 0;
    /** UNIX time of last fsync() */
    time_t lastFsync = 0;
    /** Time used by last AOF rewrite run. */
    time_t rewriteTimeLast = 0;
    /** Current AOF rewrite start time. */
    time_t rewriteTimeStart = -1;
    /** AOF buffer, written before entering the event loop */
    std::string buf;
    /** delayed AOF fsync() counter */
    uint32_t delayedFsync;
    int lastWriteError;
    /**
     * 标记同步时，是否正常写入, 如果没有正常写入，
     * 则设置该标记位，便于servercron中再次执行同步
     **/
    int lastWriteStatus;

    /** currently selected DB in AOF */
    int selectedDB = -1;

    /** 在进行aof rewrite过程中所保存的修改操作 */
    std::list<RewriteBufBlock *> rewriteBufBlocks;
};

#endif //FLYDB_AOFHANDLER_H
