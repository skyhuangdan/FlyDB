//
// Created by levy on 2019/3/12.
//

#ifndef FLYDB_REPLICATIONADAPTER_H
#define FLYDB_REPLICATIONADAPTER_H

#include <map>
#include "interface/AbstractReplicationHandler.h"
#include "ReplicationDef.h"
#include "../coordinator/interface/AbstractCoordinator.h"

typedef int stateProc();

class ReplicationStateAdapter {
public:
    ReplicationStateAdapter();
    int processState(ReplicationState state);

private:
    static int connectingStateProcess();
    static int recvPongStateProcess();
    static int recvAuthStateProcess();
    static int recvPortStateProcess();
    static int recvIPStateProcess();
    static int recvCAPAStateProcess();
    static int recvPsyncStateProcess();
    static std::map<ReplicationState, stateProc*> initStateMap();

    static std::map<ReplicationState, stateProc*> stateProcMap;
};


#endif //FLYDB_REPLICATIONADAPTER_H
