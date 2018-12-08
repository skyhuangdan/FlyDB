//
// Created by 赵立伟 on 2018/11/29.
//

#ifndef FLYDB_AOFHANDLER_H
#define FLYDB_AOFHANDLER_H

#include "interface/AbstractAOFHandler.h"

class AOFHandler : public AbstractAOFHandler {
    static AOFHandler* getInstance();

};


#endif //FLYDB_AOFHANDLER_H
