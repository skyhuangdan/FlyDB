//
// Created by 赵立伟 on 2018/9/19.
//

#ifndef FLYDB_COMMANDENTRY_H
#define FLYDB_COMMANDENTRY_H

#include <string>

#include "../flyClient/interface/AbstractFlyClient.h"
#include "../flyServer/interface/AbstractFlyServer.h"

typedef void (*commandProc)(const AbstractCoordinator*, AbstractFlyClient*);
typedef int (*getKeysProc)(struct CommandEntry *cmd,
                           FlyObj **argv,
                           int argc,
                           int *numkeys);

class CommandEntry {
public:
    CommandEntry() {

    }

    CommandEntry(commandProc proc, int flag) {
        this->proc = proc;
        this->flag = flag;
    };

    char *getName() const;
    void setName(char *name);
    commandProc getProc() const;
    void setProc(commandProc proc);
    int getArity() const;
    void setArity(int arity);
    const std::string &getSflags() const;
    void setSflags(const std::string &sflags);
    int getFlag() const;
    void setFlag(int flag);
    void setKeysProc(getKeysProc proc);
    bool isFirstKey() const;
    void setFirstKey(bool firstKey);
    bool isLastKey() const;
    void setLastKey(bool lastKey);
    uint64_t getMicroseconds() const;
    void setMicroseconds(uint64_t microseconds);
    uint64_t getCalls() const;
    void setCalls(uint64_t calls);

public:
    class Builder {
    private:
        CommandEntry *entry = new CommandEntry();

    public:
        void name(char *name) {
            entry->setName(name);
        }

        void proc(commandProc proc) {
            entry->setProc(proc);
        }

        void arity(int arity) {
            entry->setArity(arity);
        }

        void flag(int flag) {
            entry->setFlag(flag);
        }

        void keysProc (getKeysProc proc) {
            entry->setKeysProc(proc);
        }

        void firstKey(bool fistKey) {
            entry->setFirstKey(fistKey);
        }

        void lastKey(bool lastKey) {
            entry->setLastKey(lastKey);
        }

        void microseconds(uint64_t microseconds) {
            entry->setMicroseconds(microseconds);
        }

        void calls(uint64_t calls) {
            entry->setCalls(calls);
        }

        CommandEntry* build() {
            return this->entry;
        }
    };

private:
    char *name;                      /** 命令名字 */
    commandProc proc;                /** 处理函数 */
    int arity;                       /** 参数数量 */
    std::string sflags;              /** 字符串形式的flag */
    int flag;
    getKeysProc keysProc;            /** 使用函数来确定所有的keys */
    bool firstKey;                   /** 第一个参数是否是key */
    bool lastKey;                    /** 最后一个参数是否是key */
    uint64_t microseconds, calls;
};

void versionCommand(const AbstractCoordinator*, AbstractFlyClient*);
void getCommand(const AbstractCoordinator*, AbstractFlyClient*);
void setCommand(const AbstractCoordinator*, AbstractFlyClient*);
void expireCommand(const AbstractCoordinator*, AbstractFlyClient*);
void expireatCommand(const AbstractCoordinator*, AbstractFlyClient*);
void mgetCommand(const AbstractCoordinator*, AbstractFlyClient*);
void rpushCommand(const AbstractCoordinator*, AbstractFlyClient*);
void lpushCommand(const AbstractCoordinator*, AbstractFlyClient*);
void rpopCommand(const AbstractCoordinator*, AbstractFlyClient*);
void lpopCommand(const AbstractCoordinator*, AbstractFlyClient*);
void pushSortCommand(const AbstractCoordinator*, AbstractFlyClient*);
void popSortCommand(const AbstractCoordinator*, AbstractFlyClient*);
void brpopCommand(const AbstractCoordinator*, AbstractFlyClient*);
void hsetCommand(const AbstractCoordinator*, AbstractFlyClient*);
void hmgetCommand(const AbstractCoordinator*, AbstractFlyClient*);
void saveCommand(const AbstractCoordinator*, AbstractFlyClient*);
void bgsaveCommand(const AbstractCoordinator*, AbstractFlyClient*);

/**
struct CommandEntry redisCommandTable[] = {
        {"get",         getCommand,         2, "rF",  0, NULL, 1, 1, 1, 0, 0},
        {"set",         setCommand,        -3, "wm",  0, NULL, 1, 1, 1, 0, 0},
        {"setnx",       setnxCommand,       3, "wmF", 0, NULL, 1, 1, 1, 0, 0},
        {"setex",       setexCommand,       4, "wm",  0, NULL, 1, 1, 1, 0, 0},
        {"psetex",      psetexCommand,      4, "wm",  0, NULL, 1, 1, 1, 0, 0},
        {"expire",      expireCommand,      3, "wF",  0, NULL, 1, 1, 1, 0, 0},
        {"expireat",    expireatCommand,    3, "wF",  0, NULL, 1, 1, 1, 0, 0},
        {"mget",        mgetCommand,       -2, "rF",  0, NULL, 1,-1, 1, 0, 0},
        {"rpush",       rpushCommand,      -3, "wmF", 0, NULL, 1, 1, 1, 0, 0},
        {"lpush",       lpushCommand,      -3, "wmF", 0, NULL, 1, 1, 1, 0, 0},
        {"sortpush",    pushSortCommand,   -3, "wmF", 0, NULL, 1, 1, 1, 0, 0},
        {"rpop",        rpopCommand,        2, "wF",  0, NULL, 1, 1, 1, 0, 0},
        {"lpop",        lpopCommand,        2, "wF",  0, NULL, 1, 1, 1, 0, 0},
        {"sortPop",     popSortCommand,     2, "wF",  0, NULL, 1, 1, 1, 0, 0},
        {"brpop",       brpopCommand,      -3, "ws",  0, NULL, 1,-2, 1, 0, 0},
        {"hmset",       hsetCommand,       -4, "wmF", 0, NULL, 1, 1, 1, 0, 0},
        {"hmget",       hmgetCommand,      -3, "rF",  0, NULL, 1, 1, 1, 0, 0},
        {"save",        saveCommand,        1, "as",  0, NULL, 0, 0, 0, 0, 0},
        {"bgsave",      bgsaveCommand,     -1, "a",   0, NULL, 0, 0, 0, 0, 0}
};
 */

#endif //FLYDB_COMMANDENTRY_H
