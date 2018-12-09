//
// Created by 赵立伟 on 2018/12/9.
//

#ifndef FLYDB_FLYOBJENCODINGHASHTABLE_H
#define FLYDB_FLYOBJENCODINGHASHTABLE_H

#include "FlyObjEncoding.h"
#include "../../dataStructure/dict/Dict.h"

class FlyObjEncodingHashTable : public FlyObjEncoding {

public:
    void destructor(void *ptr) const {
        delete reinterpret_cast<Dict *> (ptr);
    }

};

#endif //FLYDB_FLYOBJENCODINGHASHTABLE_H
