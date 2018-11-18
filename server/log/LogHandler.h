//
// Created by 赵立伟 on 2018/11/18.
//

#ifndef FLYDB_LOGHANDLER_H
#define FLYDB_LOGHANDLER_H


#include "../FlyServer.h"

class LogHandler {
public:
    static LogHandler* getInstance(char *logfile, int syslogEnabled);
    LogHandler(char *logfile, int syslogEnabled);
    void logRaw(int level, const char *msg);

private:
    char *logfile;
    int syslogEnabled;
};


#endif //FLYDB_LOGHANDLER_H
