#ifndef DOWNLOADFILE_H
#define DOWNLOADFILE_H

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


#endif //DOWNLOADFILE_H
