//
// Created by 赵立伟 on 2018/9/18.
//

#include "FlyDB.h"
#include "dictTypeImpl/DBDictType.h"
#include "dictTypeImpl/KeyPtrDictType.h"

FlyDB::FlyDB() {
    this->dict = new Dict(new DBDictType());
    this->expires = new Dict(new KeyPtrDictType());
}