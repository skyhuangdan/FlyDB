//
// Created by 赵立伟 on 2018/12/9.
//

#ifndef FLYDB_FLYOBJENCODINGINTSET_H
#define FLYDB_FLYOBJENCODINGINTSET_H

#include "FlyObjEncoding.h"
#include "../../dataStructure/intset/IntSet.h"

class FlyObjEncodingIntSet : public FlyObjEncoding {

public:
    void destructor(void *ptr) const {
        delete reinterpret_cast<IntSet *> (ptr);
    }
};

#endif //FLYDB_FLYOBJENCODINGINTSET_H
