//
// Created by 赵立伟 on 2018/12/3.
//

#ifndef FLYDB_ABSTRACTFLYCLIENTFACTORY_H
#define FLYDB_ABSTRACTFLYCLIENTFACTORY_H

#include "../../coordinator/interface/AbstractCoordinator.h"

class AbstractFlyDB;

class AbstractFlyClientFactory {
public:
    virtual AbstractFlyClient* getFlyClient(int fd,
                                            const AbstractCoordinator *coordinator,
                                            AbstractFlyDB *flyDB) = 0;

    virtual void deleteFlyClient(AbstractFlyClient **flyClient) = 0;
};

#endif //FLYDB_ABSTRACTFLYCLIENTFACTORY_H
