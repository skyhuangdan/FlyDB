//
// Created by 赵立伟 on 2019/3/6.
//

#ifndef FLYDB_REPLICATIONHANDLER_H
#define FLYDB_REPLICATIONHANDLER_H


#include <list>
#include "interface/AbstractReplicationHandler.h"
#include "../coordinator/interface/AbstractCoordinator.h"
#include "ReplicationDef.h"

class ReplicationHandler : public AbstractReplicationHandler {
public:
    ReplicationHandler(AbstractCoordinator *coordinator);

    void unsetMaster();
    const std::string &getMasterhost() const;
    int getMasterport() const;
    bool haveMasterhost() const;

private:
    int cancelHandShake();
    void discardCachedMaster();
    void disconnectWithMaster();
    void disconnectWithSlaves();
    void shiftReplicationId();
    void randomReplicationId();
    bool inHandshakeState();
    bool abortSyncTransfer();

    /** Hostname of master */
    std::string masterhost;
    /** Port of master */
    int masterport = 6379;
    /** 主服务器可以看做一个client，因为本机也是master响应各种命令 */
    AbstractFlyClient *master = NULL;
    /** Last selected DB in replication output */
    int slaveSelDB = -1;
    ReplicationState state = REPL_STATE_NONE;
    /** current replication ID. */
    char replid[CONFIG_RUN_ID_SIZE+1];
    /** replid inherited from master*/
    char replid2[CONFIG_RUN_ID_SIZE+1];
    /** current replication offset */
    uint64_t masterReplOffset = 0;
    /** Accept offsets up to this for replid2. */
    uint64_t secondReplidOffset = 0;
    /** cached master对于本服务器来说也是个client */
    AbstractFlyClient *cachedMaster;
    /** 所有的从机*/
    std::list<AbstractFlyClient*> slaves;
    /** Master SYNC socket */
    int transferSocket = -1;
    std::string transferTempFile;

    AbstractCoordinator *coordinator;

};


#endif //FLYDB_REPLICATIONHANDLER_H
