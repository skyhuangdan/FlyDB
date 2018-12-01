//
// Created by 赵立伟 on 2018/12/1.
//

#ifndef FLYDB_FILEFIO_H
#define FLYDB_FILEFIO_H


#include <cstdio>
#include "Fio.h"

class FileFio : public Fio {
public:
    FileFio(FILE *fp);
    size_t read(struct _rio *, void *buf, size_t len);
    size_t write(struct _rio *, const void *buf, size_t len);
    int tell(struct _rio *);
    int flush(struct _rio *);
    void updateChecksum(struct _rio *, const void *buf, size_t len);

private:
    FILE *fp;
    off_t buffered; /* Bytes written since last fsync. */
    off_t autosync; /* fsync after 'autosync' bytes written. */
};


#endif //FLYDB_FILEFIO_H
