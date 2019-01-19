//
// Created by 赵立伟 on 2019/1/13.
//

#ifndef FLYDB_FLYSERVERDEF_H
#define FLYDB_FLYSERVERDEF_H

const int CONFIG_BGSAVE_RETRY_DELAY = 5;    /* 两次执行bgsave的间隔时间 */

struct saveParam {
    saveParam(time_t seconds, int changes) {
        this->seconds = seconds;
        this->changes = changes;
    }

    time_t seconds;
    int changes;
};

#endif //FLYDB_FLYSERVERDEF_H
