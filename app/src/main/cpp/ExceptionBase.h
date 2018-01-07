//
// Created by Neconspictor on 02.01.2018.
//

#ifndef BACHELOR_SEMINAR_EXCEPTIONBASE_H
#define BACHELOR_SEMINAR_EXCEPTIONBASE_H


#include <exception>
#include <string>

class ExceptionBase : public std::exception {
public:
    ExceptionBase(std::string&& msg);

    ~ExceptionBase();

    virtual const char* what() const _NOEXCEPT;

private:
    std::string msg;
};


class ConnectionException : public ExceptionBase {
public:
    ConnectionException(std::string &&msg);
    ~ConnectionException();
};

class DownloadException : public ExceptionBase {
public:
    DownloadException(std::string &&msg);
    ~DownloadException();

};

class NetworkException : public ExceptionBase {
public:
    NetworkException(std::string &&msg);
    ~NetworkException();

};

class FileNotWritableException : public ExceptionBase {
public:
    FileNotWritableException(std::string &&msg);
    ~FileNotWritableException();
};

#endif //BACHELOR_SEMINAR_EXCEPTIONBASE_H
