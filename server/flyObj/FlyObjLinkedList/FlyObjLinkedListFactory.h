//
// Created by 赵立伟 on 2018/12/9.
//

#ifndef FLYDB_FLYOBJLINKEDLISTFACTORY_H
#define FLYDB_FLYOBJLINKEDLISTFACTORY_H

#include "../interface/AbstractFlyObjFactory.h"

class FlyObjLinkedListFactory : public AbstractFlyObjFactory {
public:
    ~FlyObjLinkedListFactory();
    FlyObj* getObject();
    FlyObj* getObject(void *ptr);
};


#endif //FLYDB_FLYOBJLINKEDLISTFACTORY_H
