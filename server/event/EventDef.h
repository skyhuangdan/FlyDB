//
// Created by 赵立伟 on 2018/10/20.
//

#ifndef FLYDB_EVENTDEF_H
#define FLYDB_EVENTDEF_H


enum EventStatus {
    ES_NONE = 0,
    ES_READABLE,
    ES_WRITABLE
};

enum EventREsult {
    ER_OK = 0,
    ER_ERR
};

#endif //FLYDB_EVENTDEF_H
