//
// Created by 赵立伟 on 2018/12/9.
//

#ifndef FLYDB_FLYOBJINTFACTORY_H
#define FLYDB_FLYOBJINTFACTORY_H

#include "../interface/AbstractFlyObjFactory.h"

class FlyObjIntFactory : public AbstractFlyObjFactory {
public:

    FlyObj* getObject();
    FlyObj* getObject(void *ptr);
};


#endif //FLYDB_FLYOBJINTFACTORY_H
