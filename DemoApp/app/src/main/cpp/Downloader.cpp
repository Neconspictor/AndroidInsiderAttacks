#include "Downloader.h"
#include "Exceptions.h"
#include "Util.h"
#include <fstream>

using namespace std;
using namespace util;

#define LOG_TAG "EVIL_LIB::DOWNLOADER"

unsigned int Downloader::OPEN_MODE = std::ios::trunc | std::ios::binary;

Downloader::Downloader(Service service, std::string filePath) : service(std::move(service)) {
    downloadFile = std::shared_ptr<File>(new File(filePath));
    connPtr = std::shared_ptr<Connection>();

    isClosed = true;
    this->filePath = std::move(filePath);
}

Downloader::~Downloader() {
    logE(LOG_TAG, "Downloader::~Downloader called!");

    close();
}

void Downloader::close() {
    logE(LOG_TAG, "Downloader::close called!");

    File* file = downloadFile.get();
    Connection* connection = connPtr.get();

    if (file) file->close();

    if (connection) connection->disconnect();

    isClosed = true;
}

void Downloader::download() throw(DownloadException){
    Connection*  connection;

    isClosed = false;

    try {

        connPtr = std::shared_ptr<Connection>(new Connection(service.serverHostName.c_str(), service.port, 5));
        connection = connPtr.get();
    } catch (ConnectionException e) {
        throw DownloadException("Couldn't establish connection!");
    }

    try {

        downloadFile->openWrite(OPEN_MODE);

    } catch(FileNotWritableException e) {
        throw DownloadException("Couldn't open download file: " + std::string(e.what()));
    }

    std::ofstream* file = downloadFile->getFile().get();

    char buffer[1024];

    int fileSize = 0;

    try{
        fileSize = connection->readInt();
    } catch (NetworkException e) {

        throw DownloadException("Couldn't receive file size");
    }

    // check that a valid file size was transmitted
    if (fileSize < 0) {
        throw DownloadException("Invalid file size obtained from the server");
    }

    int total = 0;

    while(total != fileSize) {
        int count = connection->read(buffer, sizeof(buffer));

        if (count == 0) {
            throw DownloadException("Couldn't receive all bytes");
        }

        total += count;
        file->write(buffer, count);
    }

    if (file->bad()) {
        throw DownloadException("Output file is corrupted!");
    }

    //send response to the server
    int response = 666;

    try{
        connection->writeInt(response);
    } catch (NetworkException e) {
        throw DownloadException("Couldn't successfully send response!");
    }
}