//
// Created by 赵立伟 on 2018/12/9.
//

#ifndef FLYDB_FLYOBJINTSETFACTORY_H
#define FLYDB_FLYOBJINTSETFACTORY_H

#include "../interface/AbstractFlyObjFactory.h"

class FlyObjIntSetFactory : public AbstractFlyObjFactory {
public:
    ~FlyObjIntSetFactory();
    FlyObj* getObject();
    FlyObj* getObject(void *ptr);
};


#endif //FLYDB_FLYOBJINTSETFACTORY_H
