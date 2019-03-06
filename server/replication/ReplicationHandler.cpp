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

    //todo shift replication id

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

