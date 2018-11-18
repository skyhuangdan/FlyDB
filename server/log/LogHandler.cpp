//
// Created by 赵立伟 on 2018/11/18.
//

#include <syslog.h>
#include <cstdio>
#include <sys/time.h>
#include <zconf.h>
#include "LogHandler.h"
#include "../config/config.h"

LogHandler* LogHandler::getInstance(char *logfile, int syslogEnabled) {
    static LogHandler *log = NULL;
    if (NULL == log) {
        log = new LogHandler(logfile, syslogEnabled);
    }
    return log;
}

LogHandler::LogHandler(char *logfile, int syslogEnabled) {
    this->logfile = logfile;
    this->syslogEnabled = syslogEnabled;
}

void LogHandler::logRaw(int level, const char *msg) {
    const int syslogLevelMap[] = { LOG_DEBUG, LOG_INFO, LOG_NOTICE, LOG_WARNING };
    FILE *fp = '\0' == this->logfile[0] ? stdout : fopen(this->logfile, "a");
    if (NULL == fp) {
        return;
    }

    if (level & LL_RAW) {
        fprintf(fp, "%s", msg);
    } else {
        char buf[64];
        struct timeval tv;
        int role;
        const char *c = ".-*#";

        gettimeofday(&tv, NULL);
        int off = strftime(buf, sizeof(buf), "%d %b %H:%M:%S.", localtime(&tv.tv_sec));
        snprintf(buf + off, sizeof(buf) - off, "%03d", (int) tv.tv_usec / 1000);
        role = 'S';
        // 日志格式： pid:role time ./-/*/# msg
        fprintf(fp, "%d:%c %s %c %s\n", (int) getpid(), role, buf, c[level], msg);
    }

    fflush(fp);
    if (fp != stdout) {
        fclose(fp);
    }
    if (this->syslogEnabled) {
        syslog(syslogLevelMap[level], "%s", msg);
    }
}
