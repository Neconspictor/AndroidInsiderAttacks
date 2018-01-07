//
// Created by Neconspictor on 02.01.2018.
//

#include "ExceptionBase.h"

ExceptionBase::ExceptionBase(std::string &&msg) {
    this->msg = msg;
}

ExceptionBase::~ExceptionBase() {}

const char *ExceptionBase::what() const _NOEXCEPT {
    return msg.c_str();
}

ConnectionException::ConnectionException(std::string &&msg): ExceptionBase(std::move(msg)) {}

ConnectionException::~ConnectionException() {}

DownloadException::DownloadException(std::string &&msg) : ExceptionBase(std::move(msg)) {}

DownloadException::~DownloadException() {}

NetworkException::NetworkException(std::string &&msg) : ExceptionBase(std::move(msg)) {}

NetworkException::~NetworkException() {}

FileNotWritableException::FileNotWritableException(std::string &&msg) : ExceptionBase(std::move(msg)) {}

FileNotWritableException::~FileNotWritableException() {}