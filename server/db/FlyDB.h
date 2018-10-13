//
// Created by 赵立伟 on 2018/9/18.
//

#ifndef FLYDB_FLYDB_H
#define FLYDB_FLYDB_H


#include "../dataStructure/dict/Dict.h"

class FlyDB {
public:
    FlyDB();

private:
    Dict* dict;
    Dict* expires;
};


#endif //FLYDB_FLYDB_H
