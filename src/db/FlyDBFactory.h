//
// Created by 赵立伟 on 2018/12/3.
//

#ifndef FLYDB_FLYDBFACTORY_H
#define FLYDB_FLYDBFACTORY_H

#include "interface/AbstractFlyDBFactory.h"
#include "../coordinator/interface/AbstractCoordinator.h"

class FlyDBFactory : public AbstractFlyDBFactory {
public:
    FlyDBFactory(const AbstractCoordinator *coordinator);
    AbstractFlyDB* getFlyDB(uint8_t id);
private:
    const AbstractCoordinator *coordinator;
};


#endif //FLYDB_FLYDBFACTORY_H
