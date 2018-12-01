//
// Created by 赵立伟 on 2018/12/1.
//

#include "FileFio.h"

FileFio::FileFio(FILE *fp) {
    this->fp = fp;
    this->buffered = 0;
    this->autosync = 0;
}

size_t FileFio::read(struct _rio *, void *buf, size_t len) {

}

size_t FileFio::write(struct _rio *, const void *buf, size_t len) {

}

int FileFio::tell(struct _rio *) {

}

int FileFio::flush(struct _rio *) {

}

void FileFio::updateChecksum(struct _rio *, const void *buf, size_t len) {

}
