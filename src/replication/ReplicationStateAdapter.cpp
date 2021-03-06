//
// Created by levy on 2019/3/12.
//

#include "ReplicationStateAdapter.h"

extern AbstractCoordinator *coordinator;
std::map<ReplicationState, stateProc*> ReplicationStateAdapter::stateProcMap =
        ReplicationStateAdapter::initStateMap();

ReplicationStateAdapter::ReplicationStateAdapter() {
}

int ReplicationStateAdapter::processState(ReplicationState state) {
    if (stateProcMap.find(state) != stateProcMap.end()) {
        return stateProcMap.at(state)();
    }
    
    return -1;
}

int ReplicationStateAdapter::connectingStateProcess() {
    return coordinator->getReplicationHandler()->connectingStateProcess();
}

int ReplicationStateAdapter::recvPongStateProcess() {
    return coordinator->getReplicationHandler()->recvPongStateProcess();
}

int ReplicationStateAdapter::recvAuthStateProcess() {
    return coordinator->getReplicationHandler()->recvAuthStateProcess();
}

int ReplicationStateAdapter::recvPortStateProcess() {
    return coordinator->getReplicationHandler()->recvPortStateProcess();
}

int ReplicationStateAdapter::recvIPStateProcess() {
    return coordinator->getReplicationHandler()->recvIPStateProcess();
}

int ReplicationStateAdapter::recvCAPAStateProcess() {
    return coordinator->getReplicationHandler()->recvCAPAStateProcess();
}

int ReplicationStateAdapter::recvPsyncStateProcess() {
    return coordinator->getReplicationHandler()->recvPsyncStateProcess();
}

std::map<ReplicationState, stateProc*> ReplicationStateAdapter::initStateMap() {
    std::map<ReplicationState, stateProc*> stateProcMap;
    stateProcMap[REPL_STATE_CONNECTING] = connectingStateProcess;
    stateProcMap[REPL_STATE_RECEIVE_PONG] = recvPongStateProcess;
    stateProcMap[REPL_STATE_RECEIVE_AUTH] = recvAuthStateProcess;
    stateProcMap[REPL_STATE_RECEIVE_PORT] = recvPortStateProcess;
    stateProcMap[REPL_STATE_RECEIVE_IP] = recvIPStateProcess;
    stateProcMap[REPL_STATE_RECEIVE_CAPA] = recvCAPAStateProcess;
    stateProcMap[REPL_STATE_RECEIVE_PSYNC] = recvPsyncStateProcess;
    return stateProcMap;
}

