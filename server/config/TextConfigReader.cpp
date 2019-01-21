//
// Created by 赵立伟 on 2018/12/5.
//

#include <iostream>
#include "TextConfigReader.h"
#include "../../def.h"
#include "ConfigCache.h"

configMap loglevelMap[] = {
        {"debug",   LL_DEBUG},
        {"verbose", LL_VERBOSE},
        {"notice",  LL_NOTICE},
        {"warning", LL_WARNING},
        {NULL, 0}
};

configMap syslogFacilityMap[] = {
        {"user",    LOG_USER},
        {"local0",  LOG_LOCAL0},
        {"local1",  LOG_LOCAL1},
        {"local2",  LOG_LOCAL2},
        {"local3",  LOG_LOCAL3},
        {"local4",  LOG_LOCAL4},
        {"local5",  LOG_LOCAL5},
        {"local6",  LOG_LOCAL6},
        {"local7",  LOG_LOCAL7},
        {NULL, 0}
};

configMap aofFsyncEnumMap[] = {
        {"everysec", AOF_FSYNC_EVERYSEC},
        {"always", AOF_FSYNC_ALWAYS},
        {"no", AOF_FSYNC_NO},
        {NULL, 0}
};

TextConfigReader::TextConfigReader(std::string &configfile) {
    if ('-' == configfile[0] && '\0' == configfile[1]) {
        this->fp = stdin;
    } else {
        if (NULL == (this->fp = fopen(configfile.c_str(), "r"))) {
            exit(1);
        }
    }
}

ConfigCache* TextConfigReader::loadConfig() {
    char buf[CONFIG_MAX_LINE + 1];
    std::string config;

    // 从配置文件中依次读取配置 --> config
    while (fgets(buf, CONFIG_MAX_LINE + 1, fp) != NULL) {
        config += buf;
    }

    // 读取完毕，如果不是stdin，则关闭文件
    if (fp != stdin) {
        fclose(fp);
    }

    // 加载配置
    loadConfigFromString(config);

    return this->configCache;
}

void TextConfigReader::loadConfigFromString(const std::string& config) {
    // 将文件分隔成行
    std::string delim = "\n";
    std::vector<std::string> lines;
    this->miscTool->spiltString(config, delim, lines);

    // 依次处理每行
    for (auto line : lines) {
        if (0 == line.size() || '#' == line[0]) {
            continue;
        }
        loadConfigFromLineString(line);
    }
}

void TextConfigReader::loadConfigFromLineString(const std::string &line) {
    // 截取words
    std::vector<std::string> words;
    this->miscTool->spiltString(line, " ", words);
    if (0 == words.size()) {
        return;
    }

    if (0 == words[0].compare("port") && 2 == words.size()) {
        int port = atoi(words[1].c_str());
        if (0 <= port && port <= 65535) {
            this->configCache->setPort(port);
        }
    } else if (0 == words[0].compare("bind")) {
        int addressCount = words.size() - 1;
        if (addressCount > CONFIG_BINDADDR_MAX) {
            std::cout << "Too many bind addresses specified!" << std::endl;
            exit(1);
        }
        for (int j = 0; j < addressCount; j++) {
            configCache->addBindAddr(words[j + 1]);
        }
    } else if (0 == words[0].compare("unixsocket") && 2 == words.size()) {
        this->configCache->setUnixsocket(strdup(words[1].c_str()));
    } else if (0 == words[0].compare("unixsocketperm") && 2 == words.size()) {
        mode_t unixsocketperm = (mode_t) strtol(words[1].c_str(), NULL, 0);
        if (unixsocketperm > 0777) {
            std::cout << "Invalid socket file permissions" << std::endl;
            exit(1);
        }
        configCache->setUnixsocketperm(unixsocketperm);
    } else if (0 == words[0].compare("tcp-keepalive") && 2 == words.size()) {
        int tcpKeepAlive = atoi(words[1].c_str());
        if (tcpKeepAlive < 0) {
            std::cout << "Invalid tcp-keepalive value" << std::endl;
            exit(1);
        }
        configCache->setTcpKeepAlive(tcpKeepAlive);
    } else if (0 == words[0].compare("logfile") && 2 == words.size()) {
        free(configCache->getLogfile());

        char *logfile = strdup(words[1].c_str());
        if ('\0' != logfile[0]) {       // log文件名不为空
            // 尝试打开一次，查看是否可以正常打开
            FILE *logfd = fopen(logfile, "a");
            if (NULL == logfd) {
                std::cout << "Can not open log file: " << logfile << std::endl;
                exit(1);
            }
            fclose(logfd);
        }

        configCache->setLogfile(logfile);
    } else if (0 == words[0].compare("syslog-enabled") && 2 == words.size()) {
        int syslogEnabled;
        if (-1 == (syslogEnabled =
                           this->miscTool->yesnotoi(words[1].c_str()))) {
            std::cout << "syslog-enabled must be 'yes(YES)' or 'no(NO)'"
                      << std::endl;
            exit(1);
        }
        configCache->setSyslogEnabled(syslogEnabled);
    } else if (0 == words[0].compare("syslog-ident") && 2 == words.size()) {
        if (configCache->getSyslogIdent()) {
            free(configCache->getSyslogIdent());
        }
        configCache->setSyslogIdent(strdup(words[1].c_str()));
    } else if (0 == words[0].compare("loglevel") && 2 == words.size()) {
        int verbosity = configMapGetValue(loglevelMap, words[1].c_str());
        if (INT_MIN == verbosity) {
            std::cout << "Invalid log level. "
                         "Must be one of debug, verbose, notice, warning"
                      << std::endl;
            exit(1);
        }
        configCache->setVerbosity(verbosity);
    } else if (0 == words[0].compare("syslog-facility") && 2 == words.size()) {
        int syslogFacility =
                configMapGetValue(syslogFacilityMap, words[1].c_str());
        if (INT_MIN == syslogFacility) {
            std::cout << "Invalid log facility. "
                         "Must be one of USER or between LOCAL0-LOCAL7"
                      << std::endl;
            exit(1);
        }
        configCache->setSyslogFacility(syslogFacility);
    } else if (0 == words[0].compare("dbfilename") && 2 == words.size()) {
        free(configCache->getFdbFile());
        char *fdbFile = strdup(words[1].c_str());

        // 尝试打开一次，查看是否可以正常打开
        FILE *fdbfd = fopen(fdbFile, "a");
        if (NULL == fdbfd) {
            std::cout << "Can not open fdb file: " << fdbFile << std::endl;
            exit(1);
        }
        fclose(fdbfd);

        configCache->setFdbFile(fdbFile);
    } else if (0 == words[0].compare("appendonly") && 2 == words.size()) {
        int yes;
        if ((yes = this->miscTool->yesnotoi(words[1].c_str())) == -1) {
            std::cout <<  "argument must be 'yes' or 'no'";
            exit(1);
        }
        this->configCache->setAofState(yes ? AOF_ON : AOF_OFF);
    } else if (0 == words[0].compare("appendfilename") && 2 == words.size()) {
        free(configCache->getAofFile());
        char *aofFile = strdup(words[1].c_str());

        // 尝试打开一次，查看是否可以正常打开
        FILE *aoffd = fopen(aofFile, "a");
        if (NULL == aoffd) {
            std::cout << "Can not open aof file: " << aofFile << std::endl;
            exit(1);
        }
        fclose(aoffd);

        configCache->setAofFile(aofFile);
    } else if (0 == words[0].compare("aof-use-fdb-preamble")
               && 2 == words.size()) {
        int yes;
        if ((yes = this->miscTool->yesnotoi(words[1].c_str())) == -1) {
            std::cout << "argument must be 'yes' or 'no'";
            exit(1);
        }
        this->configCache->setAofUseFdbPreamble(yes ? true : false);
    } else if (0 == words[0].compare("appendfsync") && 2 == words.size()) {
        int fsync = configMapGetValue(aofFsyncEnumMap, words[1].c_str());
        if (INT_MIN == fsync) {
            std::cout << "argument must be 'no', 'always' or 'everysec'";
            exit(1);
        }

        this->configCache->setAofFsync(fsync);
    } else if (0 == words[0].compare("aof-rewrite-incremental-fsync")
               && 2 == words.size()) {
        int yes;
        if ((yes = this->miscTool->yesnotoi(words[1].c_str())) == -1) {
            std::cout <<  "argument must be 'yes' or 'no'";
            exit(1);
        }
        this->configCache->setAofRewriteIncrementalFsync(yes ? true : false);
    }
}

int TextConfigReader::configMapGetValue(configMap *config, const char *name) {
    while (config->name != NULL) {
        if (!strcasecmp(config->name, name)) {
            return config->value;
        }
        config++;
    }
    return INT_MIN;
}

