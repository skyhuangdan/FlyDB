//
// Created by 赵立伟 on 2018/11/18.
//

#include <syslog.h>
#include <cstdio>
#include <sys/time.h>
#include <zconf.h>
#include "LogHandler.h"
#include "../config/config.h"
#include "LogDef.h"

LogHandler* LogHandler::getInstance(char *logfile, int syslogEnabled, int verbosity) {
    static LogHandler *log = NULL;
    if (NULL == log) {
        log = new LogHandler(logfile, syslogEnabled, verbosity);
    }
    return log;
}

LogHandler::LogHandler(char *logfile, int syslogEnabled, int verbosity) {
    this->logfile = logfile;
    this->syslogEnabled = syslogEnabled;
    this->verbosity = verbosity;
}

void LogHandler::log(int level, const char *fmt, ...) {
    char msg[LOG_MAX_LEN];
    if (level < this->verbosity) {
        return;
    }

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);

    logRaw(level, msg);
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
