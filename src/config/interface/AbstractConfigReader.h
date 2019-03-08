//
// Created by 赵立伟 on 2018/12/5.
//

#ifndef FLYDB_ABSTRACTCONFIGREADER_H
#define FLYDB_ABSTRACTCONFIGREADER_H

#include "../ConfigCache.h"

class AbstractConfigReader {
public:
    AbstractConfigReader() {
        this->configCache = new ConfigCache();
    }

    virtual ConfigCache* loadConfig() = 0;

protected:
    ConfigCache *configCache;
};

#endif //FLYDB_ABSTRACTCONFIGREADER_H
