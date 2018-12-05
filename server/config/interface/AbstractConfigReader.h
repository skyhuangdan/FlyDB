//
// Created by 赵立伟 on 2018/12/5.
//

#ifndef FLYDB_ABSTRACTCONFIGREADER_H
#define FLYDB_ABSTRACTCONFIGREADER_H

#include "AbstractConfigCache.h"

class AbstractConfigReader {
public:
    virtual AbstractConfigCache* loadConfig() = 0;

protected:
    AbstractConfigCache *configCache;
};

#endif //FLYDB_ABSTRACTCONFIGREADER_H
