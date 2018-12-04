//
// Created by 赵立伟 on 2018/12/3.
//

#ifndef FLYDB_ABSTRACTFLYDBFACTORY_H
#define FLYDB_ABSTRACTFLYDBFACTORY_H

#include "AbstractFlyDB.h"

class AbstractFlyDBFactory {
public:
    virtual AbstractFlyDB* getFlyDB() = 0;

};

#endif //FLYDB_ABSTRACTFLYDBFACTORY_H
