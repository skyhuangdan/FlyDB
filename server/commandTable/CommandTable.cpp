//
// Created by 赵立伟 on 2018/10/15.
//

#include <iostream>
#include "CommandTable.h"
#include "CommandEntry.h"
#include "../utils/MiscTool.h"
#include "../log/FileLogFactory.h"
#include "../dataStructure/dict/Dict.cpp"
#include "CommandTableDef.h"

CommandTable::CommandTable(const AbstractCoordinator* coordinator) {
    this->commands = new Dict<std::string, CommandEntry>();
    this->logHandler = logFactory->getLogger();
    this->coordinator = coordinator;
    this->populateCommand();
}

CommandTable::~CommandTable() {
    delete this->commands;
}

void CommandTable::populateCommand() {
    int num = flyDBCommandTable.size();
    for (int i = 0; i < num; i++) {
        CommandEntry *entry = flyDBCommandTable[i];
        const char *f = entry->getSflags().c_str();
        while (*f != '\0') {
            switch (*f) {
                case 'w':
                    entry->addFlag(CMD_WRITE);
                    break;
                case 'r':
                    entry->addFlag(CMD_READONLY);
                    break;
                case 'm':
                    entry->addFlag(CMD_DENYOOM);
                    break;
                case 'a':
                    entry->addFlag(CMD_ADMIN);
                    break;
                case 'p':
                    entry->addFlag(CMD_PUBSUB);
                    break;
                case 's':
                    entry->addFlag(CMD_NOSCRIPT);
                    break;
                case 'R':
                    entry->addFlag(CMD_RANDOM);
                    break;
                case 'S':
                    entry->addFlag(CMD_SORT_FOR_SCRIPT);
                    break;
                case 'l':
                    entry->addFlag(CMD_LOADING);
                    break;
                case 't':
                    entry->addFlag(CMD_STALE);
                    break;
                case 'M':
                    entry->addFlag(CMD_SKIP_MONITOR); break;
                case 'k':
                    entry->addFlag(CMD_ASKING);
                    break;
                case 'F':
                    entry->addFlag(CMD_FAST);
                    break;
                default:
                    exit(1);
            }
            f++;
        }
        this->commands->addEntry(entry->getName(), *entry);
    }
}

int CommandTable::dealWithCommand(AbstractFlyClient* flyClient) {
    std::string *command = reinterpret_cast<std::string*>
            (flyClient->getArgv()[0]->getPtr());
    DictEntry<std::string, CommandEntry>* dictEntry =
            this->commands->findEntry(*command);
    if (NULL == dictEntry) {
        this->logHandler->logDebug("wrong command type: %s", command);
        return -1;
    }
    dictEntry->getVal().getProc()(this->coordinator, flyClient);

    /** 添加命令序列到相应的缓冲中 */
    this->feedAppendOnlyFile(dictEntry,
                             flyClient->getDbid(),
                             flyClient->getArgv(),
                             flyClient->getArgc());

    return 1;
}

void CommandTable::feedAppendOnlyFile(
        DictEntry<std::string, CommandEntry>* entry,
        int dbid,
        std::shared_ptr<FlyObj> *argv,
        int argc) {
    AbstractAOFHandler *aofHandler = coordinator->getAofHandler();
    std::string buf;

    if (aofHandler->getSelectedDB() != dbid) {
        char seldb[10];
        char selstr[100];
        snprintf(seldb, sizeof(seldb), "%d", dbid);
        snprintf(selstr, sizeof(seldb),
                 "*2\r\n$6\r\nselect\r\n$%d\r\n%s\r\n",
                 strlen(seldb), seldb);
        buf += selstr;
        aofHandler->setSelectedDB(dbid);
    }

    /** 获取遵循RESP协议的命令串 */
    buf += this->catAppendOnlyGenericCommand(argv, argc);

    /** 如果buf长度不为0，则假如相应的buf中 */
    if (buf.length() > 0) {
        /**
         * 如果当前aof处于开启状态，加入到this->buf里面，
         * 后续再根据fsyc策略（EVERYSECOND/always)调用flush写入aof文件
         **/
        if (aofHandler->IsStateOn()) {
            aofHandler->addToBuf(buf);
        }

        /**
         * 如果是写入，才将其假如append block list(后续要保存到aof文件中)
         * 这里和this->buf不同，因为this->buf是当前执行的aof命令，不管是什么命令
         * 都要执行完然后返回给客户端响应，但是这里是存入aof文件中作用于持久化的，
         * 所以对于非写入命令无需假如appen block list
         **/
        if (entry->getVal().IsWrite()) {
            /** 如果正在进行background aof，将该buf写入append buf list中 */
            if (aofHandler->haveChildPid()) {
                aofHandler->rewriteBufferAppend(buf);
            }
        }
    }
}

std::string CommandTable::catAppendOnlyGenericCommand(
        std::shared_ptr<FlyObj> *argv,
        int argc) {
    char buf[32];
    char argcstr[10];
    snprintf(argcstr, sizeof(argcstr), "%d", argc);
    snprintf(buf, sizeof(buf), "*$d\r\n", argcstr);
    std::string res = buf;

    /** 写入各个参数 */
    for (int i = 0; i < argc; i++) {
        std::string *tempArgv =
                reinterpret_cast<std::string*>(argv[i]->getPtr());
        snprintf(buf, sizeof(buf), "$%d\r\n%s\r\n",
                 tempArgv->length(), tempArgv->c_str());
        res += buf;
    }

    return res;
}
