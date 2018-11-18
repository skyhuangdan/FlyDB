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

char* LogHandler::logfile = NULL;
int LogHandler::syslogEnabled = 0;
int LogHandler::verbosity = 0;

LogHandler* LogHandler::getInstance() {
    static LogHandler *log = NULL;
    if (NULL == log) {
        log = new LogHandler();
    }
    return log;
}

void LogHandler::init(char *logfile, int syslogEnabled, int verbosity) {
    LogHandler::logfile = logfile;
    LogHandler::syslogEnabled = syslogEnabled;
    LogHandler::verbosity = verbosity;
}

void LogHandler::logRaw(int level, const char *msg) {
    const int syslogLevelMap[] = { LOG_DEBUG, LOG_INFO, LOG_NOTICE, LOG_WARNING };
    FILE *fp = '\0' == logfile[0] ? stdout : fopen(logfile, "a");
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
    if (syslogEnabled) {
        syslog(syslogLevelMap[level], "%s", msg);
    }
}

void LogHandler::log(int level, const char *fmt, ...) {
    if (level & 0xff < verbosity) {
        return;
    }

    char msg[LOG_MAX_LEN];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);

    logRaw(level, msg);
}

void LogHandler::logDebug(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    log(LL_DEBUG, fmt, ap);
    va_end(ap);
}

void LogHandler::logVerbose(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    log(LL_VERBOSE, fmt, ap);
    va_end(ap);
}

void LogHandler::logNotice(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    log(LL_NOTICE, fmt, ap);
    va_end(ap);
}

void LogHandler::logWarning(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    log(LL_WARNING, fmt, ap);
    va_end(ap);
}

void LogHandler::log(int level, const char *fmt, va_list ap) {
    char msg[LOG_MAX_LEN];
    if (level & 0xff < verbosity) {
        return;
    }

    vsnprintf(msg, sizeof(msg), fmt, ap);
    logRaw(level, msg);
}

