//
// Created by 赵立伟 on 2018/12/3.
//

#include "FlyClientFactory.h"
#include "FlyClient.h"

AbstractFlyClient* FlyClientFactory::getFlyClient(
        int fd,
        const AbstractCoordinator *coordinator,
        AbstractFlyDB *flyDB) {
    return new FlyClient(fd, coordinator, flyDB);
}

void FlyClientFactory::deleteFlyClient(AbstractFlyClient **flyClient) {
    if (NULL == flyClient) {
        return;
    }

    delete *flyClient;
    *flyClient = NULL;
}
