//
// Created by 赵立伟 on 2018/12/5.
//

#ifndef FLYDB_CONFIGREADER_H
#define FLYDB_CONFIGREADER_H

#include <string>
#include "interface/AbstractConfigReader.h"
#include "interface/AbstractConfigCache.h"
#include "../utils/MiscTool.h"
#include "../../def.h"

class TextConfigReader : public AbstractConfigReader {
public:
    TextConfigReader(AbstractConfigCache *configCache,
                      const std::string &fileName);
    AbstractConfigCache* loadConfig();

private:
    void loadConfigFromString(const std::string& config);
    void loadConfigFromLineString(const std::string &line);
    int configMapGetValue(configMap *config, const char *name);

    std::string fileName;

    MiscTool *miscTool;
};

#endif //FLYDB_CONFIGREADER_H
