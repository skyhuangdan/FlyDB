//
// Created by levy on 2019/3/12.
//

#include "ReplicationStateAdapter.h"

extern AbstractCoordinator *coordinator;
std::map<ReplicationState, stateProc*> ReplicationStateAdapter::stateProcMap =
        ReplicationStateAdapter::initStateMap();

ReplicationStateAdapter::ReplicationStateAdapter() {
}

void ReplicationStateAdapter::processState(ReplicationState state) {
    if (stateProcMap.find(state) != stateProcMap.end()) {
        stateProcMap.at(state)();
    }
}

void ReplicationStateAdapter::connectingStateProcess() {
    coordinator->getReplicationHandler()->connectingStateProcess();
}

void ReplicationStateAdapter::recvPongStateProcess() {
    coordinator->getReplicationHandler()->recvPongStateProcess();
}

void ReplicationStateAdapter::sendAuthStateProcess() {
    coordinator->getReplicationHandler()->sendAuthStateProcess();
}

void ReplicationStateAdapter::recvAuthStateProcess() {
    coordinator->getReplicationHandler()->recvAuthStateProcess();
}

void ReplicationStateAdapter::sendPortStateProcess() {
    coordinator->getReplicationHandler()->sendPortStateProcess();
}

void ReplicationStateAdapter::recvPortStateProcess() {
    coordinator->getReplicationHandler()->recvPortStateProcess();
}

void ReplicationStateAdapter::sendIPStateProcess() {
    coordinator->getReplicationHandler()->sendIPStateProcess();
}

void ReplicationStateAdapter::recvIPStateProcess() {
    coordinator->getReplicationHandler()->recvIPStateProcess();
}

void ReplicationStateAdapter::sendCAPAStateProcess() {
    coordinator->getReplicationHandler()->sendCAPAStateProcess();
}

void ReplicationStateAdapter::recvCAPAStateProcess() {
    coordinator->getReplicationHandler()->recvCAPAStateProcess();
}

void ReplicationStateAdapter::sendPsyncStateProcess() {
    coordinator->getReplicationHandler()->sendPsyncStateProcess();
}

void ReplicationStateAdapter::recvPsyncStateProcess() {
    coordinator->getReplicationHandler()->recvPsyncStateProcess();
}

std::map<ReplicationState, stateProc*> ReplicationStateAdapter::initStateMap() {
    std::map<ReplicationState, stateProc*> stateProcMap;
    stateProcMap[REPL_STATE_CONNECTING] = connectingStateProcess;
    stateProcMap[REPL_STATE_RECEIVE_PONG] = recvPongStateProcess;
    stateProcMap[REPL_STATE_SEND_AUTH] = sendAuthStateProcess;
    stateProcMap[REPL_STATE_RECEIVE_AUTH] = recvAuthStateProcess;
    stateProcMap[REPL_STATE_SEND_PORT] = sendPortStateProcess;
    stateProcMap[REPL_STATE_RECEIVE_PORT] = recvPortStateProcess;
    stateProcMap[REPL_STATE_SEND_IP] = sendIPStateProcess;
    stateProcMap[REPL_STATE_RECEIVE_IP] = recvIPStateProcess;
    stateProcMap[REPL_STATE_SEND_CAPA] = sendCAPAStateProcess;
    stateProcMap[REPL_STATE_RECEIVE_CAPA] = recvCAPAStateProcess;
    stateProcMap[REPL_STATE_SEND_PSYNC] = sendPsyncStateProcess;
    stateProcMap[REPL_STATE_RECEIVE_PSYNC] = recvPsyncStateProcess;
    return stateProcMap;
}

