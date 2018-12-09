//
// Created by 赵立伟 on 2018/12/9.
//

#ifndef FLYDB_FLYOBJENCODING_H
#define FLYDB_FLYOBJENCODING_H

#include "../FlyObjDef.h"

class FlyObjEncoding {
public:
    virtual void destructor(void *ptr) const = 0;

    virtual FlyObjEncode getEncode() const {
        return this->encode;
    };

protected:
    FlyObjEncode encode;
};

#endif //FLYDB_FLYOBJENCODING_H
