//
// Created by 赵立伟 on 2018/12/5.
//

#ifndef FLYDB_CONFIGREADER_H
#define FLYDB_CONFIGREADER_H

#include <string>
#include "interface/AbstractConfigReader.h"
#include "../utils/MiscTool.h"
#include "../def.h"
#include "ConfigCache.h"

struct configMap {
    const char *name;
    const int value;
};

class TextConfigReader : public AbstractConfigReader {
public:
    TextConfigReader(std::string &configfile);
    ConfigCache* loadConfig();

private:
    void loadConfigFromString(const std::string& config);
    void loadConfigFromLineString(const std::string &line);
    int configMapGetValue(configMap *config, const char *name);

    FILE *fp;
    MiscTool *miscTool;
};

#endif //FLYDB_CONFIGREADER_H
