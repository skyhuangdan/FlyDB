//
// Created by 赵立伟 on 2018/11/4.
//

#ifndef FLYDB_STRINGTOOL_H
#define FLYDB_STRINGTOOL_H

#include <string>
#include <vector>
#include <zconf.h>
#include <sstream>
#include <sys/time.h>

class MiscTool {
public:
    static MiscTool* getInstance () {
        static MiscTool *instance = NULL;
        if (NULL == instance) {
            instance = new MiscTool();
        }
        return instance;
    }

    void spiltString(const std::string &str,
                     const std::string &delim,
                     std::vector<std::string> &res) {
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

    int getAbsolutePath(const std::string &fileName, std::string &absPath) {
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

    int64_t string2int64(std::string str, int64_t &num) {
        for (int i = 0; i < str.size(); i++) {
            if (str[i] > '9' || str[i] < '0') {
                // 对于非数字，如果是第一位并且符号是'-'，则是正常的
                if (0 == i && ('-' == str[i] || '+' == str[i])) {
                    continue;
                }
                return -1;
            }
        }

        std::istringstream is(str);
        is >> num;

        return 1;
    }

    int yesnotoi(const char *s) {
        if (!strcasecmp(s, "yes")) {
            return 1;
        } else if (!strcasecmp(s, "no")) {
            return 0;
        } else return -1;
    }

    uint64_t ustime(void) {
        struct timeval tv;
        uint64_t ust;

        gettimeofday(&tv, NULL);
        ust = ((uint64_t)tv.tv_sec) * 1000000;
        ust += tv.tv_usec;
        return ust;
    }

    // Return the UNIX time in milliseconds
    uint64_t mstime(void) {
        return ustime() / 1000;
    }

    uint64_t memtoll(const char *p, char *err) {
        const char *u;
        char buf[128];
        uint64_t mul = 1;
        uint64_t val = 0;
        unsigned int digits;

        if (NULL != err) {
            *err = 0;
        }

        /* Search the first non digit character. */
        u = p;
        if (*u == '-') {
            u++;
        }
        while (*u && isdigit(*u)) {
            u++;
        }

        if (*u == '\0' || !strcasecmp(u, "b")) {
            mul = 1;
        } else if (!strcasecmp(u, "k")) {
            mul = 1000;
        } else if (!strcasecmp(u, "kb")) {
            mul = 1024;
        } else if (!strcasecmp(u, "m")) {
            mul = 1000*1000;
        } else if (!strcasecmp(u, "mb")) {
            mul = 1024*1024;
        } else if (!strcasecmp(u, "g")) {
            mul = 1000L*1000*1000;
        } else if (!strcasecmp(u, "gb")) {
            mul = 1024L*1024*1024;
        } else {
            if (NULL != err) {
                *err = 1;
            }
            return 0;
        }

        /* Copy the digits into a buffer, we'll use strtoll() to convert
         * the digit (without the unit) into a number. */
        digits = u - p;
        if (digits >= sizeof(buf)) {
            if (NULL != err) {
                *err = 1;
            }
            return 0;
        }
        memcpy(buf, p, digits);
        buf[digits] = '\0';

        char *endptr;
        errno = 0;
        val = strtoll(buf, &endptr, 10);
        if ((val == 0 && errno == EINVAL) || *endptr != '\0') {
            if (NULL != err) {
                *err = 1;
            }
            return 0;
        }

        return val * mul;
    }

    void getRandomHexChars(char *p, unsigned int len) {
        char *charset = "0123456789abcdef";

        /* Global state. */
        static int seed_initialized = 0;
        static unsigned char seed[20]; /* The SHA1 seed, from /dev/urandom. */
        static uint64_t counter = 0; /* The counter we hash with the seed. */

        if (!seed_initialized) {
            /* Initialize a seed and use SHA1 in counter mode, where we hash
             * the same seed with a progressive counter. For the goals of this
             * function we just need non-colliding strings, there are no
             * cryptographic security needs. */
            FILE *fp = fopen("/dev/urandom", "r");
            if (NULL != fp && fread(seed, sizeof(seed), 1, fp) == 1) {
                seed_initialized = 1;
            }

            if (NULL != fp) {
                fclose(fp);
            }
        }

        if (seed_initialized) {
            while(len > 0) {
                unsigned char digest[20];
                uint8_t copylen = len > 20 ? 20 : len;
                /**
                SHA1_CTX ctx;

                SHA1Init(&ctx);
                SHA1Update(&ctx, seed, sizeof(seed));
                SHA1Update(&ctx, (unsigned char*)&counter, sizeof(counter));
                SHA1Final(digest, &ctx);
                counter++;
                 */

                memcpy(p, digest, copylen);
                /* Convert to hex digits. */
                for (int j = 0; j < copylen; j++) {
                    p[j] = charset[p[j] & 0x0F];
                }
                len -= copylen;
                p += copylen;
            }
        } else {
            /* If we can't read from /dev/urandom, do some reasonable effort
             * in order to create some entropy, since this function is used to
             * generate run_id and cluster instance IDs */
            char *x = p;
            unsigned int l = len;
            struct timeval tv;
            pid_t pid = getpid();

            /* Use time and PID to fill the initial array. */
            gettimeofday(&tv, NULL);
            if (l >= sizeof(tv.tv_usec)) {
                memcpy(x, &tv.tv_usec, sizeof(tv.tv_usec));
                l -= sizeof(tv.tv_usec);
                x += sizeof(tv.tv_usec);
            }
            if (l >= sizeof(tv.tv_sec)) {
                memcpy(x, &tv.tv_sec, sizeof(tv.tv_sec));
                l -= sizeof(tv.tv_sec);
                x += sizeof(tv.tv_sec);
            }
            if (l >= sizeof(pid)) {
                memcpy(x, &pid, sizeof(pid));
                l -= sizeof(pid);
                x += sizeof(pid);
            }
            /* Finally xor it with rand() output, that was already seeded with
             * time() at startup, and convert to hex digits. */
            for (int j = 0; j < len; j++) {
                p[j] ^= rand();
                p[j] = charset[p[j] & 0x0F];
            }
        }
    }

private:
    MiscTool() {
    }
};

#endif //FLYDB_STRINGTOOL_H
