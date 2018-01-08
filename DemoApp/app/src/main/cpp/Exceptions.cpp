//
// Created by Neconspictor on 02.01.2018.
//

#include "Exceptions.h"

using namespace std;

ExceptionBase::ExceptionBase(string msg) {
    this->msg = msg;
}

ExceptionBase::~ExceptionBase() {}

const char *ExceptionBase::what() const _NOEXCEPT {
    return msg.c_str();
}

ConnectionException::ConnectionException(string msg): ExceptionBase(move(msg)) {}

ConnectionException::~ConnectionException() {}

DownloadException::DownloadException(string msg) : ExceptionBase(move(msg)) {}

DownloadException::~DownloadException() {}

NetworkException::NetworkException(string msg) : ExceptionBase(move(msg)) {}

NetworkException::~NetworkException() {}

FileNotWritableException::FileNotWritableException(string msg) : ExceptionBase(move(msg)) {}

FileNotWritableException::~FileNotWritableException() {}

NoSuchFileException::NoSuchFileException(string msg) : ExceptionBase(move(msg)) {}

NoSuchFileException::~NoSuchFileException() {}

ClassNotFoundException::ClassNotFoundException(std::string msg) : ExceptionBase(msg) {}

ClassNotFoundException::~ClassNotFoundException() {}