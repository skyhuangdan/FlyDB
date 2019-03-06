//
// Created by 赵立伟 on 2019/3/6.
//

#ifndef FLYDB_REPLICATIONDEF_H
#define FLYDB_REPLICATIONDEF_H

enum ReplicationState {
    REPL_STATE_NONE = 0,            /** No active replication */
    REPL_STATE_CONNECT = 1,         /** Must connect to master */
    REPL_STATE_CONNECTING = 2,      /** Connecting to master */
    /** --握手状态，必须保持有序-- */
    REPL_STATE_RECEIVE_PONG = 3,    /** Wait for PING reply */
    REPL_STATE_SEND_AUTH = 4,       /** Send AUTH to master */
    REPL_STATE_RECEIVE_AUTH = 5,    /** Wait for AUTH reply */
    REPL_STATE_SEND_PORT = 6,       /** Send REPLCONF listening-port */
    REPL_STATE_RECEIVE_PORT = 7,    /** Wait for REPLCONF reply */
    REPL_STATE_SEND_IP = 8,         /** Send REPLCONF ip-address */
    REPL_STATE_RECEIVE_IP = 9,      /** Wait for REPLCONF reply */
    REPL_STATE_SEND_CAPA = 10,      /** Send REPLCONF capa */
    REPL_STATE_RECEIVE_CAPA = 11,   /** Wait for REPLCONF reply */
    REPL_STATE_SEND_PSYNC = 12,     /** Send PSYNC */
    REPL_STATE_RECEIVE_PSYNC = 13,  /** Wait for PSYNC reply */
    /** --- 握手结束的状态 --- */
    REPL_STATE_TRANSFER = 14,       /** Receiving .rdb from master */
    REPL_STATE_CONNECTED = 15       /** Connected to master */
};

#endif //FLYDB_REPLICATIONDEF_H
