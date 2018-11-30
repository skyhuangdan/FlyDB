//
// Created by 赵立伟 on 2018/11/29.
//

#include <cstdio>
#include "FDBHandler.h"

FDBHandler* FDBHandler::getInstance() {
    static FDBHandler* instance = NULL;
    if (NULL == instance) {
        instance = new FDBHandler();
    }

    return instance;
}
