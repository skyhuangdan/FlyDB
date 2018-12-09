//
// Created by 赵立伟 on 2018/12/9.
//

#ifndef FLYDB_FLYENCODINGSTRING_H
#define FLYDB_FLYENCODINGSTRING_H

#include <string>
#include "FlyObjEncoding.h"

class FlyObjEncodingString : public FlyObjEncoding {

public:
    void destructor(void *ptr) const {
        delete reinterpret_cast<std::string *> (ptr);
    }

};

#endif //FLYDB_FLYENCODINGSTRING_H
