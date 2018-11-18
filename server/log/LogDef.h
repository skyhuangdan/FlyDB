//
// Created by 赵立伟 on 2018/11/18.
//

#ifndef FLYDB_LOGDEF_H
#define FLYDB_LOGDEF_H

#define LOG_MAX_LEN    1024             // log最大长度
enum logLevel {
    LL_DEBUG,
    LL_VERBOSE,
    LL_NOTICE,
    LL_WARNING,
    LL_RAW = (1 << 10)
};

#endif //FLYDB_LOGDEF_H
