//
// Created by 赵立伟 on 2019/1/13.
//

#include "Pipe.h"
#include "PipeDef.h"

Pipe::Pipe(const AbstractCoordinator *coordinator,
           int infoPipe[2]) {
    this->coordinator = coordinator;

    /** initialize pipe */
    int count = sizeof(this->infoPipe) / sizeof(int);
    for (int i = 0; i < count; i++) {
        this->infoPipe[i] = infoPipe[i];
    }

    /** open pipe */
    this->open();
}

Pipe::~Pipe() {
    /** close pipe */
    this->closeAll();
}

void Pipe::sendInfo(PipeType ptype) {
    if (-1 == this->infoPipe[1]) {
        return;
    }

    /** send pipe data */
    PipeData* pipeData = PipeData::Builder().processType(ptype)
            .magic(CHILD_INFO_MAGIC)
            .build();
    write(this->infoPipe[1], pipeData, sizeof(PipeData));
}

PipeCowBytes* Pipe::recvInfo(void) {
    PipeCowBytes *cowBytes = new PipeCowBytes();

    if (-1 == this->infoPipe[0]) {
        return cowBytes;
    }

    PipeData* pipeData = PipeData::Builder().build();

    /** recv pipe data */
    ssize_t wlen = sizeof(PipeData);
    if (wlen == read(this->infoPipe[0], pipeData, wlen)
            && CHILD_INFO_MAGIC == pipeData->magic) {
        if (CHILD_INFO_TYPE_AOF == pipeData->processType) {
            cowBytes->aofCowBytes = pipeData->cowSize;
        } else {
            cowBytes->rdbCowBytes = pipeData->cowSize;
        }
    }

    return cowBytes;
}

void Pipe::open(void) {
    if (-1 == pipe(infoPipe)) {
        this->closeAll();
        return;
    }

    if (-1 == coordinator->getNetHandler()->setBlock(
            NULL, this->infoPipe[0], 0)) {
        this->closeAll();
    }
}

void Pipe::closeAll(void) {
    if (-1 != this->infoPipe[0] || -1 != this->infoPipe[1]) {
        close(this->infoPipe[0]);
        close(this->infoPipe[1]);
        this->infoPipe[0] = -1;
        this->infoPipe[1] = -1;
    }
}
