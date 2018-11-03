//
// Created by 赵立伟 on 2018/9/18.
//

#include <iostream>
#include "FlyServer.h"
#include "commandTable/CommandEntry.h"
#include "config.h"

void FlyServer::init() {
    // init db array
    for (int i = 0; i < DB_NUM; i++) {
        this->dbArray[i] = new FlyDB();
        if (NULL == this->dbArray.at(i)) {
            std::cout << "error to create FlyDB[" << i << "]" << std::endl;
            exit(1);
        }
    }

    // init command table
    this->commandTable = new CommandTable(this);

    // server端口
    this->port = CONFIG_DEFAULT_SERVER_PORT;

    // 设置最大客户端数量
    setMaxClientLimit();

    // 时间循环处理器
    this->eventLoop = new EventLoop(this, this->maxClients + CONFIG_FDSET_INCR);
    this->eventLoop->createTimeEvent(1, serverCron, NULL, NULL);

    // serverCron运行频率
    this->hz = CONFIG_CRON_HZ;

    return;
}

int FlyServer::getPID() {
    return this->pid;
}

FlyDB* FlyServer::getDB(int dbID) {
    return this->dbArray.at(dbID);
}

std::string FlyServer::getVersion() {
    return this->version;
}

int FlyServer::dealWithCommand(std::string* command) {
    return this->commandTable->dealWithCommand(command);
}

void FlyServer::setMaxClientLimit() {
    this->maxClients = CONFIG_DEFAULT_MAX_CLIENTS;
    int maxFiles = this->maxClients + CONFIG_MIN_RESERVED_FDS;
    rlimit limit;

    // 获取当前进程可打开的最大文件描述符
    if (getrlimit(RLIMIT_NOFILE, &limit) == -1) {
        // 如果获取失败, 按照进程中最大文件数量为1024计算(内核默认1024), 重置maxClients
        this->maxClients = 1024 - CONFIG_MIN_RESERVED_FDS;
    } else {
        int softLimit = limit.rlim_cur;
        // 如果soft limit小于maxfiles, 则尝试增大soft limit, 并重置maxClients
        if (softLimit < maxFiles) {
            int curLimit = maxFiles;
            int decrStep = 16;

            // 逐步试探提高softlimit
            while (curLimit > softLimit) {
                limit.rlim_cur = curLimit;
                if (setrlimit(RLIMIT_NOFILE, &limit) != -1) {
                    break;
                }
                curLimit -= decrStep;
            }
            if (curLimit < softLimit) {
                curLimit = softLimit;
            }

            // 如果当前文件数量限制小于最小保留文件数，程序退出
            if (curLimit <= CONFIG_MIN_RESERVED_FDS) {
                exit(1);
            }

            this->maxClients = curLimit - CONFIG_MIN_RESERVED_FDS;
        }
    }
}

void FlyServer::eventMain() {
    this->eventLoop->eventMain();
}

int FlyServer::getHz() const {
    return hz;
}

void FlyServer::setHz(int hz) {
    FlyServer::hz = hz;
}

void FlyServer::loadConfig(std::string fileName, std::string option) {
    char buf[CONFIG_MAX_LINE + 1];
    std::string config;

    if (fileName.length() != 0) {
        FILE *fp = NULL;
        if ('-' == fileName[0] && '\0' == fileName[1]) {
            fp = stdin;
        } else {
            if (NULL == (fp = fopen(fileName.c_str(), "r"))) {
                exit(1);
            }
        }

        // 从配置文件中依次读取配置 --> config
        while (fgets(buf, CONFIG_MAX_LINE + 1, fp) != NULL) {
            config += buf;
        }

        // 读取完毕，如果不是stdin，则关闭文件
        if (fp != stdin) {
            fclose(fp);
        }
    }

    config += "\n" + option;
    loadConfigFromString(config);
}

void FlyServer::loadConfigFromString(const std::string& config) {
    int pos = 0, find_pos = 0;
    int len = config.length();
    std::string subStr;

    std::string delim = "\t\r\n";
    while (pos < len) {
        // 获取sub str
        find_pos = config.find(delim, pos);
        if (find_pos < 0) {
            subStr = config.substr(pos, len - pos);
        } else {
            subStr = config.substr(pos, find_pos - pos);
        }

        // 从该子串获取config
        if ('#' != subStr[0] && '\0' != subStr[0]) {
            loadConfigFromSubString(subStr);
        }

        // 继续处理下一个子串
        pos = find_pos;
    }
}

void FlyServer::loadConfigFromSubString(const std::string& subStr) {
}

int FlyServer::listenToPort() {
}

int serverCron(EventLoop *eventLoop, uint64_t id, void *clientData) {
    if (NULL == eventLoop || NULL == eventLoop->getFlyServer()) {
        return 0;
    }

    static int times = 0;
    std::cout << "serverCron is running " << times++ << " times!" << std::endl;

    return 1000 / eventLoop->getFlyServer()->getHz();
}
