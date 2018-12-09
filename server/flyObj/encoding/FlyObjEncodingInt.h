//
// Created by 赵立伟 on 2018/12/9.
//

#ifndef FLYDB_FLYOBJENCODINGINT_H
#define FLYDB_FLYOBJENCODINGINT_H

#include "FlyObjEncoding.h"

class FlyObjEncodingInt : public FlyObjEncoding {

public:
    void destructor(void *ptr) const {
        delete reinterpret_cast<int *> (ptr);
    }
};

#endif //FLYDB_FLYOBJENCODINGINT_H
