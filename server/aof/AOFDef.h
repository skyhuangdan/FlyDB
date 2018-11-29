//
// Created by 赵立伟 on 2018/11/29.
//

#ifndef FLYDB_AOFDEF_H
#define FLYDB_AOFDEF_H

enum AOFState {
    AOF_OFF = 0,
    AOF_ON,
    AOF_WAIT_REWRITE
};

#endif //FLYDB_AOFDEF_H
