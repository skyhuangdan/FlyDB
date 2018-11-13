//
// Created by 赵立伟 on 2018/11/4.
//

#ifndef FLYDB_STRINGTOOL_H
#define FLYDB_STRINGTOOL_H

#include <string>
#include <vector>
#include <zconf.h>
#include <sstream>

class MiscTool {
public:
    static MiscTool* getInstance () {
        static MiscTool *instance = NULL;
        if (NULL == instance) {
            instance = new MiscTool();
        }
        return instance;
    }

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

    static int getAbsolutePath(const std::string &fileName, std::string &absPath) {
        if (0 == fileName.size()) {
            return -1;
        }

        // 如果已经是绝对地址，则直接返回
        if ('/' == fileName[0]) {
            absPath = fileName;
        }

        // 获取当前路径
        char cwd[1024];
        if (NULL == getcwd(cwd, sizeof(cwd))) {
            return -1;
        }

        // 获取绝对路径
        absPath = cwd;
        if (0 == absPath.size()) {
            return -1;
        }
        if (absPath[absPath.size() - 1] != '/') {
            absPath += '/';
        }
        absPath += fileName;

        // 尝试打开一次文件，用于查看是否存在该文件
        FILE *fp;
        if (NULL == (fp = fopen(fileName.c_str(), "r"))) {
            return -1;
        }
        fclose(fp);

        return 1;
    }

    static int64_t string2int64(std::string str) {
        int64_t result;
        std::istringstream is(str);
        is >> result;
        return result;
    }

};

#endif //FLYDB_STRINGTOOL_H
