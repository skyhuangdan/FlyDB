//
// Created by 赵立伟 on 2019/3/6.
//

#ifndef FLYDB_REPLICATIONHANDLER_H
#define FLYDB_REPLICATIONHANDLER_H

#include <list>
#include <map>
#include "interface/AbstractReplicationHandler.h"
#include "../coordinator/interface/AbstractCoordinator.h"
#include "ReplicationDef.h"
#include "ReplicationStateAdapter.h"

class ReplicationHandler : public AbstractReplicationHandler {
public:
    ReplicationHandler(AbstractCoordinator *coordinator);

    void unsetMaster();
    void setMaster(std::string ip, int port);
    const std::string &getMasterhost() const;
    int getMasterport() const;
    bool haveMasterhost() const;
    void cron();
    void syncWithMaster(int fd,
                        std::shared_ptr<AbstractFlyClient> flyClient,
                        int mask);

    /** state process procs */
    int connectingStateProcess();
    int recvPongStateProcess();
    int recvAuthStateProcess();
    int recvPortStateProcess();
    int recvIPStateProcess();
    int recvCAPAStateProcess();
    int recvPsyncStateProcess();

private:
    int cancelHandShake();
    void discardCachedMaster();
    void disconnectWithMaster();
    void disconnectWithSlaves();
    void shiftReplicationId();
    void randomReplicationId();
    bool inHandshakeState();
    bool abortSyncTransfer();
    void cacheMasterUsingMyself();
    int connectWithMaster();
    void sendAck();
    char* recvSynchronousCommand(int fd, ...);
    bool sendSynchronousCommand(int fd, ...);
    int slaveTryPartialResynchronization(int fd, bool readReply);

    static void syncWithMasterStatic(
            const AbstractCoordinator *coorinator,
            int fd,
            std::shared_ptr<AbstractFlyClient> flyClient,
            int mask);


    /** Hostname of master */
    std::string masterhost;
    /** Port of master */
    int masterport = 6379;
    /** 主服务器可以看做一个client，因为本机也是master响应各种命令 */
    std::shared_ptr<AbstractFlyClient> master = NULL;
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
    std::shared_ptr<AbstractFlyClient> cachedMaster;
    /** 所有的从机*/
    std::list<std::shared_ptr<AbstractFlyClient>> slaves;
    /** Master SYNC socket */
    int transferSocket = -1;
    /** 接收PSYNC传输数据的临时文件 */
    std::string transferTempFile;
    /** 与master断开连接的时间 */
    time_t masterDownSince = 0;
    /** 上次io时间 */
    time_t transferLastIO = 0;
    /** 上次交互时间 */
    time_t lastInteraction = 0;
    /** 超时时间(秒) */
    int timeout = CONFIG_DEFAULT_REPL_TIMEOUT;
    /** replication offset */
    int64_t offset = 0;
    /** 鉴权 */
    std::string masterAuth;
    std::string masterUser;
    /** 发送给master保存的slave ip和port */
    int slaveAnnouncePort;
    std::string slaveAnnounceIP;
    /** 同步io操作超时时间 */
    int syncioTimeout = CONFIG_REPL_SYNCIO_TIMEOUT;

    AbstractCoordinator *coordinator;
    AbstractLogHandler *logHandler;
    ReplicationStateAdapter *stateAdapter;
};


#endif //FLYDB_REPLICATIONHANDLER_H
