//
// Created by 赵立伟 on 2018/12/3.
//

#ifndef FLYDB_ABSTRACTFLYCLIENTFACTORY_H
#define FLYDB_ABSTRACTFLYCLIENTFACTORY_H

#include "../../coordinator/interface/AbstractCoordinator.h"

class AbstractFlyClientFactory {
public:
    virtual AbstractFlyClient* getFlyClient(
            int fd, const AbstractCoordinator *coordinator) = 0;
};

#endif //FLYDB_ABSTRACTFLYCLIENTFACTORY_H
