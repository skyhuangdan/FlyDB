//
// Created by 赵立伟 on 2018/12/9.
//

#ifndef FLYDB_FLYOBJSTRINGFACTORY_H
#define FLYDB_FLYOBJSTRINGFACTORY_H

#include "../interface/AbstractFlyObjFactory.h"

class FlyObjStringFactory : public AbstractFlyObjFactory {
public:
    ~FlyObjStringFactory();
    FlyObj* getObject(void *ptr);
    FlyObj* getObject();
};


#endif //FLYDB_FLYOBJSTRINGFACTORY_H
