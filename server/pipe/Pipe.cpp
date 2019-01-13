//
// Created by 赵立伟 on 2019/1/13.
//

#include "Pipe.h"
#include "PipeDef.h"

Pipe::Pipe(const AbstractCoordinator *coordinator) {
    this->coordinator = coordinator;
}

Pipe::~Pipe() {
    /** close pipe */
    this->closeAll();
}

void Pipe::sendInfo(PipeType ptype, size_t cowSize) {
    if (-1 == this->infoPipe[1]) {
        return;
    }

    /** send pipe data */
    PipeData* pipeData = PipeData::Builder().processType(ptype)
            .magic(CHILD_INFO_MAGIC)
            .cowSize(cowSize)
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
        if (PIPE_TYPE_AOF == pipeData->processType) {
            cowBytes->aofCowBytes = pipeData->cowSize;
        } else {
            cowBytes->rdbCowBytes = pipeData->cowSize;
        }
    }

    return cowBytes;
}

void Pipe::open(void) {
    if (-1 == pipe(this->infoPipe)) {
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
