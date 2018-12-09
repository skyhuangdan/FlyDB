//
// Created by 赵立伟 on 2018/12/9.
//

#ifndef FLYDB_FLYOBJENCODINGLINKEDLIST_H
#define FLYDB_FLYOBJENCODINGLINKEDLIST_H

#include <string>
#include <list>
#include "FlyObjEncoding.h"

class FlyObjEncodingLinkedList : public FlyObjEncoding {

public:
    void destructor(void *ptr) const {
        delete reinterpret_cast<std::list<std::string> *> (ptr);
    }
};

#endif //FLYDB_FLYOBJENCODINGLINKEDLIST_H
