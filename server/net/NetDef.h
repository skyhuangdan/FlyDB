//
// Created by 赵立伟 on 2018/11/3.
//

#ifndef FLYDB_NETDEF_H
#define FLYDB_NETDEF_H

const int NET_ERR_LEN = 256;
const int NET_NONE = 0;
const int NET_IP_ONLY = 1<<0;

enum NetConnectFlag {
    ANET_CONNECT_NONE = 0,
    ANET_CONNECT_NONBLOCK = 1,
    ANET_CONNECT_BE_BINDING = 2     /* Best effort binding. */
};

#endif //FLYDB_NETDEF_H
