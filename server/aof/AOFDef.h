//
// Created by 赵立伟 on 2018/11/29.
//

#ifndef FLYDB_AOFDEF_H
#define FLYDB_AOFDEF_H

#include <cstdint>

enum AOFState {
    AOF_OFF = 0,
    AOF_ON,
    AOF_WAIT_REWRITE
};

enum AOFFsyncEnum {
    AOF_FSYNC_NO = 0,
    AOF_FSYNC_ALWAYS,
    AOF_FSYNC_EVERYSEC
};

const uint32_t AOF_AUTOSYNC_BYTES = 1024 * 1024 * 32;    /** fdatasync every 32MB */
const int AOF_WRITE_LOG_ERROR_RATE = 30;                 /** Seconds between errors logging. */

#endif //FLYDB_AOFDEF_H
