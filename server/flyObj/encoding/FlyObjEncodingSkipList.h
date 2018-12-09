//
// Created by 赵立伟 on 2018/12/9.
//

#ifndef FLYDB_FLYOBJENCODINGSKIPLIST_H
#define FLYDB_FLYOBJENCODINGSKIPLIST_H

#include "FlyObjEncoding.h"
#include "../../dataStructure/skiplist/SkipList.h"

class FlyObjEncodingSkipList : public FlyObjEncoding {

public:
    void destructor(void *ptr) const {
        delete reinterpret_cast<SkipList *> (ptr);
    }

};

#endif //FLYDB_FLYOBJENCODINGSKIPLIST_H
