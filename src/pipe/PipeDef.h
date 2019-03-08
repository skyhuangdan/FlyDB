//
// Created by 赵立伟 on 2019/1/13.
//

#ifndef FLYDB_PIPEDEF_H
#define FLYDB_PIPEDEF_H

#include <cstdint>
#include <cstdio>

const uint64_t CHILD_INFO_MAGIC = 0xC17DDA7A12345678LL;

enum PipeType {
    PIPE_TYPE_RDB = 0,
    PIPE_TYPE_AOF = 1
};

struct PipeData {
    PipeType processType;           /** AOF or RDB child? */
    size_t cowSize;            /** Copy on write size. */
    uint64_t magic;            /** Magic value to make sure data is valid. */

    struct Builder {
        public:
        Builder() {
            this->pipeData = new PipeData();
        }

        Builder& processType(PipeType processType) {
            this->pipeData->processType = processType;
            return *this;
        }

        Builder& cowSize(size_t cowSize) {
            this->pipeData->cowSize = cowSize;
            return *this;
        }

        Builder& magic(uint64_t magic) {
            this->pipeData->magic = magic;
            return *this;
        }

        PipeData* build() {
            return this->pipeData;
        }

        private:
        PipeData * pipeData;
    };
};

struct PipeCowBytes {
    size_t rdbCowBytes;
    size_t aofCowBytes;
};

#endif //FLYDB_PIPEDEF_H
