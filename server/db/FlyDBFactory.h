//
// Created by 赵立伟 on 2018/12/3.
//

#ifndef FLYDB_FLYDBFACTORY_H
#define FLYDB_FLYDBFACTORY_H

#include "interface/AbstractFlyDBFactory.h"

class FlyDBFactory : public AbstractFlyDBFactory {
public:

    AbstractFlyDB* getFlyDB();
};


#endif //FLYDB_FLYDBFACTORY_H
