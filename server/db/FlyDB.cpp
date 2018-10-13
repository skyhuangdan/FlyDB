//
// Created by 赵立伟 on 2018/9/18.
//

#include "FlyDB.h"
#include "DBDictType.h"

FlyDB::FlyDB() {
    this->dict = new Dict(DBDictType());
    this->expires = new Dict(DBDictType());
}