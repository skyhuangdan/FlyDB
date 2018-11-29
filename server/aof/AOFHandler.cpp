//
// Created by 赵立伟 on 2018/11/29.
//

#include <cstdio>
#include "AOFHandler.h"

AOFHandler* AOFHandler::getInstance() {
    static AOFHandler *instance = NULL;
    if (NULL == instance) {
        instance = new AOFHandler();
    }

    return instance;
}
