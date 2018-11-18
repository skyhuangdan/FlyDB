//
// Created by 赵立伟 on 2018/11/18.
//

#ifndef FLYDB_LOGHANDLER_H
#define FLYDB_LOGHANDLER_H

#include "../FlyServer.h"

class LogHandler {
public:
    static LogHandler* getInstance(char *logfile, int syslogEnabled, int verbosity);
    LogHandler(char *logfile, int syslogEnabled, int verbosity);
    void logDebug(const char *fmt, ...);
    void logVerbose(const char *fmt, ...);
    void logNotice(const char *fmt, ...);
    void logWarning(const char *fmt, ...);
    void logRaw(int level, const char *msg);
    void log(int level, const char *fmt, ...);

private:
    void log(int level, const char *fmt, va_list ap);

    char *logfile;
    int syslogEnabled;
    int verbosity;
};


#endif //FLYDB_LOGHANDLER_H
