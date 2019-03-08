//
// Created by 赵立伟 on 2019/2/28.
//

#ifndef FLYDB_FLYDBDEF_H
#define FLYDB_FLYDBDEF_H

/** 每次cron遍历的db数量 */
const int CRON_DBS_PER_CALL = 16;
/** Loopkups per loop. */
const int ACTIVE_EXPIRE_CYCLE_LOOKUPS_PER_LOOP = 20;
/** Microseconds */
const int ACTIVE_EXPIRE_CYCLE_FAST_DURATION = 1000;
/** CPU max % for keys collection */
const int ACTIVE_EXPIRE_CYCLE_SLOW_TIME_PERC = 25;

enum ActiveExpireCycleMode {
    ACTIVE_EXPIRE_CYCLE_SLOW = 0,
    ACTIVE_EXPIRE_CYCLE_FAST = 1
};

#endif //FLYDB_FLYDBDEF_H
