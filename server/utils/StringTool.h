//
// Created by 赵立伟 on 2018/11/4.
//

#ifndef FLYDB_STRINGTOOL_H
#define FLYDB_STRINGTOOL_H

#include <string>
#include <vector>

class StringTool {
public:
    static void spiltString(const std::string &str, const std::string &delim, std::vector<std::string> &res) {
        int pos = 0, findPos = 0;
        int len = str.length();
        int delimLen = delim.length();

        while (pos < len) {
            while (pos < len && ' ' == str[pos]) {
                pos++;
            }
            if (pos >= len) {
                return;
            }

            findPos = str.find(delim, pos);
            // 如果需要处理字符的特殊格式，则在这里处理
            if (findPos < 0) {
                res.push_back(str.substr(pos, len - pos));
                return;
            } else {
                res.push_back(str.substr(pos, findPos - pos));
            }

            pos = findPos + delimLen;
        }
    }
};

#endif //FLYDB_STRINGTOOL_H
