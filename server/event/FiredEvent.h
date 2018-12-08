//
// Created by 赵立伟 on 2018/10/27.
//

#ifndef FLYDB_FIREDEVENT_H
#define FLYDB_FIREDEVENT_H

struct FiredEvent {
public:
    FiredEvent(int fd, int mask);
    int getFd() const;
    void setFd(int fd);
    int getMask() const;
    void setMask(int mask);

private:
    int fd;
    int mask;
};


#endif //FLYDB_FIREDEVENT_H
