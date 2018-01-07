//
// Created by Neconspictor on 02.01.2018.
//

#ifndef BACHELOR_SEMINAR_DOWNLOADFILE_H
#define BACHELOR_SEMINAR_DOWNLOADFILE_H

#include "File.h"
#include "Connection.h"
#include "Service.h"

#include <string>

class Downloader {
public:

    Downloader(Service service, std::string filePath);
    ~Downloader();

    void download() throw(DownloadException);

    void close();

private:
    std::shared_ptr<File> downloadFile;
    std::shared_ptr<Connection> connPtr;

    static unsigned int OPEN_MODE;

    Service service;
    bool isClosed;
    std::string filePath;
};


#endif //BACHELOR_SEMINAR_DOWNLOADFILE_H
