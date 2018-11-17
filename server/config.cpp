//
// Created by 赵立伟 on 2018/11/17.
//

#include <strings.h>

int yesnotoi(const char *s) {
    if (!strcasecmp(s, "yes")) {
        return 1;
    } else if (!strcasecmp(s, "no")) {
        return 0;
    } else return -1;
}
