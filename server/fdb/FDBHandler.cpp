//
// Created by 赵立伟 on 2018/11/29.
//

#include <cstdio>
#include "FDBHandler.h"
#include "../io/FileFio.h"

FDBHandler::FDBHandler(char *filename) {
    this->filename = filename;
}

int FDBHandler::load(fdbSaveInfo &fdbSaveInfo) {
    // open fdb file with read premission
    FILE *fp;
    if (NULL == (fp = fopen(this->filename, "r"))) {
        return -1;
    }

    // start to load
    this->startToLoad();

    // do real load
    Fio *fio = new FileFio(fp);
    doRealLoad(fio, fdbSaveInfo);

    // 读取完毕
    fclose(fp);
    this->stopLoad();
}

void FDBHandler::doRealLoad(Fio *fio, fdbSaveInfo &saveInfo) {

}

void FDBHandler::startToLoad() {
    this->loading = true;
    this->loadingStartTime = time(NULL);
}

void FDBHandler::stopLoad() {
    this->loading = false;
}