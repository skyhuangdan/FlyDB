//
// Created by 赵立伟 on 2019/3/10.
//

#ifndef FLYDB_STRINGFIO_H
#define FLYDB_STRINGFIO_H

#include "base/Fio.h"

class StringFio : public Fio {
public:
    StringFio(const std::string &str);
    int tell();                         // get offset
    int flush();                        // flush data to device

private:
    size_t baseread(void *buf, size_t readLen);
    size_t basewrite(const void *buf, size_t len);

    std::string str;
};


#endif //FLYDB_STRINGFIO_H
