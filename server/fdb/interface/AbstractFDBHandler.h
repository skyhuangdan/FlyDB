//
// Created by 赵立伟 on 2018/12/8.
//

#ifndef FLYDB_ABSTRACTFDBHANDLER_H
#define FLYDB_ABSTRACTFDBHANDLER_H

#include "../FDBDef.h"
#include "../../io/interface/Fio.h"

class FlyObj;

class AbstractFDBHandler {
public:
    virtual int load(FDBSaveInfo *fdbSaveInfo) = 0;

    virtual int save() = 0;

    virtual int backgroundSave() = 0;

    virtual void backgroundSaveDone(int exitCode, int bySignal) = 0;

    virtual int loadFromFio(Fio *fio, FDBSaveInfo *saveInfo) = 0;

    virtual int saveKeyValuePair(Fio *fio,
                                 std::string &key,
                                 FlyObj *val,
                                 int64_t expireTime) = 0;

    virtual ssize_t saveRawString(Fio *fio, const std::string &str) = 0;

    virtual void deleteTempFile(pid_t pid) = 0;

    virtual pid_t getFdbChildPid() const = 0;

    virtual bool haveFdbChildPid() const = 0;

    virtual void setFdbChildPid(pid_t fdbChildPid) = 0;

    virtual bool isFdbBGSaveScheduled() const = 0;

    virtual void setFdbBGSaveScheduled(bool fdbBGSaveScheduled) = 0;

    virtual bool canBgsaveNow() = 0;

    virtual time_t getLastSaveTime() const = 0;

    virtual bool lastSaveTimeGapGreaterThan(time_t gap) const = 0;

    virtual void setLastSaveTime(time_t lastSaveTime) = 0;

    virtual void setBgsaveLastTryTime(time_t bgsaveLastTryTime) = 0;

    virtual int getLastBgsaveStatus() const = 0;

    virtual void setLastBgsaveStatus(int lastBgsaveStatus) = 0;

    virtual int getFdbChildType() const = 0;

    virtual void setFdbDiskChildType() = 0;

    virtual void setFdbNoneChildType() = 0;

    virtual void setFdbBgSaveDone(int status) = 0;

    virtual void setFdbSaveDone() = 0;

    virtual int getSaveParamsCount() const = 0;

    virtual const saveParam* getSaveParam(int pos) const = 0;

    virtual uint64_t getDirty() const = 0;

    virtual uint64_t addDirty(uint64_t count) = 0;
};

#endif //FLYDB_ABSTRACTFDBHANDLER_H
