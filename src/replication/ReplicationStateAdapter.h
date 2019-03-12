//
// Created by levy on 2019/3/12.
//

#ifndef FLYDB_REPLICATIONADAPTER_H
#define FLYDB_REPLICATIONADAPTER_H

#include "interface/AbstractReplicationHandler.h"
#include "ReplicationDef.h"

class ReplicationStateAdapter {
public:
    ReplicationStateAdapter(AbstractReplicationHandler *handler);
    void processState(ReplicationState state);

    AbstractReplicationHandler *handler;
};


#endif //FLYDB_REPLICATIONADAPTER_H
