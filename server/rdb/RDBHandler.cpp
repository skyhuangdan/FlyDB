//
// Created by 赵立伟 on 2018/11/29.
//

#include <cstdio>
#include "RDBHandler.h"

RDBHandler* RDBHandler::getInstance() {
    static RDBHandler* instance = NULL;
    if (NULL == instance) {
        instance = new RDBHandler();
    }

    return instance;
}
