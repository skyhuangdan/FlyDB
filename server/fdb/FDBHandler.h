//
// Created by 赵立伟 on 2018/11/29.
//

#ifndef FLYDB_FDBHANDLER_H
#define FLYDB_FDBHANDLER_H

#include <ctime>
#include <cstdint>
#include <string>
#include "FDBDef.h"
#include "../io/base/Fio.h"
#include "interface/AbstractFDBHandler.h"
#include "../coordinator/interface/AbstractCoordinator.h"
#include "../log/interface/AbstractLogHandler.h"
#include "../utils/EndianConvTool.h"

class FDBHandler : public AbstractFDBHandler {
public:
    FDBHandler(const AbstractCoordinator *coordinator,
               char *filename,
               uint64_t maxProcessingChunk);
    ~FDBHandler();
    int load(FDBSaveInfo *saveInfo);
    int loadFromFile(FILE *fp, FDBSaveInfo *saveInfo);
    int save();
    int saveToFio(Fio *fio, int flag, const FDBSaveInfo *saveInfo);
    int backgroundSave();
    void backgroundSaveDone(int exitCode, int bySignal);
    int saveKeyValuePair(Fio *fio,
                         std::string key,
                         std::shared_ptr<FlyObj> val,
                         int64_t expireTime);
    ssize_t saveRawString(Fio *fio, const std::string &str);
    void deleteTempFile(pid_t pid);
    pid_t getChildPid() const;
    void setChildPid(pid_t fdbChildPid);
    bool haveChildPid() const;
    bool isBGSaveScheduled() const;
    void setBGSaveScheduled(bool fdbBGSaveScheduled);
    time_t getLastSaveTime() const;
    bool lastSaveTimeGapGreaterThan(time_t gap) const;
    void setLastSaveTime(time_t lastSaveTime);
    void setBgsaveLastTryTime(time_t bgsaveLastTryTime);
    bool canBgsaveNow();
    int getLastBgsaveStatus() const;
    void setLastBgsaveStatus(int lastBgsaveStatus);
    int getChildType() const;
    void setDiskChildType();
    void setNoneChildType();
    void setBgSaveDone(int status);
    void setSaveDone();
    int getSaveParamsCount() const;
    const saveParam* getSaveParam(int pos) const;
    uint64_t getDirty() const;
    uint64_t addDirty(uint64_t count);

private:
    static int dbScan(void *priv, std::string key, std::shared_ptr<FlyObj> val);
    static int dictSaveScan(void *priv, std::string key, std::string val);
    static void skipListSaveProc(void *priv, const std::string &obj);
    ssize_t saveNum(Fio *fio, uint64_t len);
    ssize_t saveObject(Fio *fio, std::shared_ptr<FlyObj> obj);
    int saveInfoAuxFields(Fio *fio,
                          int flags,
                          const FDBSaveInfo *saveInfo);
    int saveAuxFieldStrStr(Fio *fio,
                           const std::string &key,
                           const std::string &val);
    int saveAuxFieldStrInt(Fio *fio,
                           const std::string &key,
                           int64_t val);
    int saveAuxField(Fio *fio,
                     const std::string &key,
                     const std::string &val);
    int saveMillisecondTime(Fio *fio, int64_t t);
    int saveType(Fio *fio, unsigned char type);
    int loadFromFio(Fio *fio, FDBSaveInfo *saveInfo);
    char loadChar(Fio *fio);
    uint8_t loadUint8(Fio *fio);
    time_t loadTime(Fio *fio);
    uint64_t loadMillisecondTime(Fio *fio);
    int loadNum(Fio *fio, int *encoded);
    int loadNumByRef(Fio *fio, int *encoded, uint64_t *lenptr);
    std::shared_ptr<FlyObj> loadStringObject(Fio *fio);
    std::string* loadStringPlain(Fio *fio);
    void* genericLoadString(Fio *fio, size_t *lenptr);
    std::shared_ptr<FlyObj> genericLoadStringObject(Fio *fio, size_t *lenptr);
    void* loadIntegerObject(Fio *fio, int encoding, int flag, size_t *lenptr);
    void* loadLzfStringObject(Fio *fio, int flag, size_t *lenptr);
    int checkHeader(Fio *fio);
    void checkThenExit(int linenum, char *reason, ...);
    std::shared_ptr<FlyObj> loadObject(int type, Fio *fio);
    void initSaveParams();

    char *filename;
    off_t maxProcessingChunk = 0;
    uint64_t cksum = 0;
    uint64_t dirty = 0;
    pid_t childPid = -1;
    bool bgSaveScheduled = false;
    time_t lastSaveTime = time(NULL);
    time_t bgsaveLastTryTime = time(NULL);
    int lastBgsaveStatus = 1;
    int childType = RDB_CHILD_TYPE_NONE;
    std::vector<saveParam> saveParams;

    AbstractLogHandler *logHandler;
    const AbstractCoordinator *coordinator;
    EndianConvTool *endianConvTool;
};


#endif //FLYDB_FDBHANDLER_H
