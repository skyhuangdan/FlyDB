//
// Created by 赵立伟 on 2018/12/3.
//

#ifndef FLYDB_ABSTRACTFLYOBJFACTORY_H
#define FLYDB_ABSTRACTFLYOBJFACTORY_H

#include "FlyObj.h"

class AbstractFlyObjFactory {
public:
    virtual FlyObj* getObject() = 0;
    virtual FlyObj* getObject(void *ptr) = 0;
};

#endif //FLYDB_ABSTRACTFLYOBJFACTORY_H
