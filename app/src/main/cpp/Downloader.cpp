//
// Created by Neconspictor on 02.01.2018.
//

#include "Downloader.h"
#include "ExceptionBase.h"
#include "Util.h"
#include <fstream>

unsigned int Downloader::OPEN_MODE = std::ios::trunc | std::ios::binary;

Downloader::Downloader(std::string serverName, int port, std::string filePath) {
    downloadFile = std::shared_ptr<File>(new File(std::move(filePath)));
    connPtr = std::shared_ptr<Connection>();

    this->serverName = serverName;
    this->port = port;

    isClosed = true;
    this->filePath = std::move(filePath);
}

Downloader::~Downloader() {
    LOGE("Downloader::~Downloader called!");
    close();
}

void Downloader::close() {
    LOGE("Downloader::close called!");
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

        connPtr = std::shared_ptr<Connection>(new Connection(serverName.c_str(), port));
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