//
// Created by levy on 2019/3/12.
//

#include "ReplicationStateAdapter.h"

ReplicationStateAdapter::ReplicationStateAdapter(AbstractReplicationHandler *handler) {
    this->handler = handler;
}

void ReplicationStateAdapter::processState(ReplicationState state) {
    switch (state) {
        case REPL_STATE_CONNECTING:
            this->handler->connectingStateProcess();
            break;
        case REPL_STATE_RECEIVE_PONG:
            this->handler->recvPongStateProcess();
            break;
        case REPL_STATE_SEND_AUTH:
            this->handler->sendAuthStateProcess();
            break;
        case REPL_STATE_RECEIVE_AUTH:
            this->handler->recvAuthStateProcess();
            break;
        case REPL_STATE_SEND_PORT:
            this->handler->sendPortStateProcess();
            break;
        case REPL_STATE_RECEIVE_PORT:
            this->handler->recvPortStateProcess();
            break;
        case REPL_STATE_SEND_IP:
            this->handler->sendIPStateProcess();
            break;
        case REPL_STATE_RECEIVE_IP:
            this->handler->recvIPStateProcess();
            break;
        case REPL_STATE_SEND_CAPA:
            this->handler->sendCAPAStateProcess();
            break;
        case REPL_STATE_RECEIVE_CAPA:
            this->handler->recvCAPAStateProcess();
            break;
        case REPL_STATE_SEND_PSYNC:
            this->handler->sendPsyncStateProcess();
            break;
        case REPL_STATE_RECEIVE_PSYNC:
            this->handler->recvPsyncStateProcess();
            break;
        default:
            return;
    }
}
