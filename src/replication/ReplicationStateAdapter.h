//
// Created by levy on 2019/3/12.
//

#ifndef FLYDB_REPLICATIONADAPTER_H
#define FLYDB_REPLICATIONADAPTER_H

#include <map>
#include "interface/AbstractReplicationHandler.h"
#include "ReplicationDef.h"
#include "../coordinator/interface/AbstractCoordinator.h"

typedef void stateProc();

class ReplicationStateAdapter {
public:
    ReplicationStateAdapter();
    void processState(ReplicationState state);

private:
    static void connectingStateProcess();
    static void recvPongStateProcess();
    static void sendAuthStateProcess();
    static void recvAuthStateProcess();
    static void sendPortStateProcess();
    static void recvPortStateProcess();
    static void sendIPStateProcess();
    static void recvIPStateProcess();
    static void sendCAPAStateProcess();
    static void recvCAPAStateProcess();
    static void sendPsyncStateProcess();
    static void recvPsyncStateProcess();
    static std::map<ReplicationState, stateProc*> initStateMap();

    static std::map<ReplicationState, stateProc*> stateProcMap;
};


#endif //FLYDB_REPLICATIONADAPTER_H
