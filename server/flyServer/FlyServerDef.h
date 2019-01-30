//
// Created by 赵立伟 on 2019/1/13.
//

#ifndef FLYDB_FLYSERVERDEF_H
#define FLYDB_FLYSERVERDEF_H

const int CONFIG_BGSAVE_RETRY_DELAY = 5;    /* 两次执行bgsave的间隔时间 */

/** SHUTDOWN flags */
enum ShutDownFlag {
    SHUTDOWN_NOFLAGS = 0,      /** No flags. */
    SHUTDOWN_SAVE = 1,         /** Force SAVE on SHUTDOWN even if no save points are configured. */
    SHUTDOWN_NOSAVE = 2        /** Don't SAVE on SHUTDOWN. */
};

#endif //FLYDB_FLYSERVERDEF_H
