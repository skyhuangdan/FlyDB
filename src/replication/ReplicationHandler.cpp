//
// Created by 赵立伟 on 2019/3/6.
//

#include "ReplicationHandler.h"

ReplicationHandler::ReplicationHandler(AbstractCoordinator *coordinator) {
    this->coordinator = coordinator;
}

void ReplicationHandler::unsetMaster() {
    if (!this->haveMasterhost()) {
        return;
    }

    /** 清理masterhost */
    this->masterhost.clear();

    /** shift replication id */
    this->shiftReplicationId();

    /** 删除master, todo: 使用ClientFactory的freeClient */
    if (NULL != this->master) {
        delete this->master;
        this->master = NULL;
    }

    this->discardCachedMaster();
    this->cancelHandShake();
    this->disconnectSlaves();

    /** replication state */
    this->state = REPL_STATE_NONE;

    /**
     * 这里将slaveSelDB设置为1，为了在下次full resync时强加一个SELECT命令
     **/
    this->slaveSelDB = -1;
}

const std::string &ReplicationHandler::getMasterhost() const {
    return this->masterhost;
}

int ReplicationHandler::getMasterport() const {
    return this->masterport;
}

bool ReplicationHandler::haveMasterhost() const {
    return !this->masterhost.empty();
}

void ReplicationHandler::cancelHandShake() {

}

void ReplicationHandler::disconnectSlaves() {

}

void ReplicationHandler::discardCachedMaster() {

}

/**
 * 本机从主切换到从，replication id切换：
 *  1.将replication id/offset存入replication id2/offset2
 *  2.replid2 = replid + 1，这样在切换成slave时，slave取其已经拥有的字节的下一个字节
 *  3.重新生成replication id
 **/
void ReplicationHandler::shiftReplicationId() {
    memcpy(this->replid2, this->replid, sizeof(this->replid));
    this->secondReplidOffset = this->masterReplOffset + 1;
    this->randomReplicationId();
}

void ReplicationHandler::randomReplicationId() {
    uint8_t len = sizeof(this->replid);
    miscTool->getRandomHexChars(this->replid, len);
    this->replid[len] = '\0';
}
