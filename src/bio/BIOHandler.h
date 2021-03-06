//
// Created by 赵立伟 on 2019/1/20.
//

#ifndef FLYDB_BIOHANDLER_H
#define FLYDB_BIOHANDLER_H

#include <pthread.h>
#include <list>
#include <array>
#include "BIODef.h"
#include "../coordinator/interface/AbstractCoordinator.h"
#include "interface/AbstractBIOHandler.h"
#include "BIOJob.h"

/**
 * background I/O
 */
class BIOHandler : public AbstractBIOHandler {
public:
    uint64_t getPendingJobCount(int type);
    uint64_t waitStep(int type);
    void createBackgroundJob(int type, void *arg1, void *arg2, void *arg3);
    void killThreads(void);

private:
    static bool init();
    static void initStackSize(pthread_attr_t *attr);
    static void *processBackgroundJobs(void *arg);

    static bool __init;
    static std::array<pthread_t, BIO_NUM_OPS> threads;
    static std::array<pthread_mutex_t, BIO_NUM_OPS> mutex;
    static std::array<pthread_cond_t, BIO_NUM_OPS> newjobCond;
    static std::array<pthread_cond_t, BIO_NUM_OPS> stepCond;
    /** 表示某种job code下的jobs列表 */
    static std::array<std::list<BIOJob*>, BIO_NUM_OPS> jobs;
    /** 表示某类型下悬挂了多少个job */
    static std::array<uint64_t, BIO_NUM_OPS> pending;

    static AbstractLogHandler *logHandler;
};


#endif //FLYDB_BIOHANDLER_H
