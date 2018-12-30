//
// Created by 赵立伟 on 2018/12/9.
//

#ifndef FLYDB_FLYOBJSKIPLISTFACTORY_H
#define FLYDB_FLYOBJSKIPLISTFACTORY_H

#include "../interface/AbstractFlyObjFactory.h"

class FlyObjSkipListFactory : public AbstractFlyObjFactory {
public:
    ~FlyObjSkipListFactory();
    FlyObj* getObject();
    FlyObj* getObject(void *ptr);
};


#endif //FLYDB_FLYOBJSKIPLISTFACTORY_H
