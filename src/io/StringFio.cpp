//
// Created by 赵立伟 on 2019/3/10.
//

#include "StringFio.h"

StringFio::StringFio(const std::string &str) {
    this->str = str;
}

StringFio::StringFio() {

}

int StringFio::tell() {
    return 0;
}

int StringFio::flush() {
    return 0;
}

std::string StringFio::getStr() const {
    return this->str;
}

size_t StringFio::baseread(void *buf, size_t readLen) {
    snprintf(reinterpret_cast<char*>(buf), readLen, "%s", this->str.c_str());
    int len = this->str.length();
    if (len > readLen) {
        this->str = this->str.substr(readLen, len);
        return readLen;
    } else {
        this->str.clear();
        return len;
    }
}

size_t StringFio::basewrite(const void *buf, size_t writeLen) {
    if (NULL == buf) {
        return 0;
    }

    const char *chars = reinterpret_cast<const char*>(buf);
    int charlen = strlen(chars);
    int realLen = charlen > writeLen ? writeLen : charlen;

    strncpy((char*)this->str.c_str(), chars, realLen);
    return realLen;
}
