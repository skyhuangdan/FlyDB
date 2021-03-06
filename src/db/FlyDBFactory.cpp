//
// Created by 赵立伟 on 2018/12/3.
//

#include "FlyDBFactory.h"
#include "FlyDB.h"

FlyDBFactory::FlyDBFactory(const AbstractCoordinator *coordinator) {
    this->coordinator = coordinator;
}

AbstractFlyDB* FlyDBFactory::getFlyDB(uint8_t id) {
    return new FlyDB(this->coordinator, id);
}
