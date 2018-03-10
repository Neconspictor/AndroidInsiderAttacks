#ifndef EXCEPTIONBASE_H
#define EXCEPTIONBASE_H


#include <exception>
#include <string>

class ExceptionBase : public std::exception {
public:
    ExceptionBase(std::string msg);

    ~ExceptionBase();

    virtual const char* what() const _NOEXCEPT;

private:
    std::string msg;
};


class ConnectionException : public ExceptionBase {
public:
    ConnectionException(std::string msg);
    ~ConnectionException();
};

class DownloadException : public ExceptionBase {
public:
    DownloadException(std::string msg);
    ~DownloadException();

};

class NetworkException : public ExceptionBase {
public:
    NetworkException(std::string msg);
    ~NetworkException();

};

class NoSuchFileException : public ExceptionBase {
public:
    NoSuchFileException(std::string msg);
    ~NoSuchFileException();

};

class FileNotWritableException : public ExceptionBase {
public:
    FileNotWritableException(std::string msg);
    ~FileNotWritableException();
};

class ClassNotFoundException : public ExceptionBase {
public:
    ClassNotFoundException(std::string msg);
    ~ClassNotFoundException();
};

class AllocationException : public ExceptionBase {
public:
    AllocationException(std::string msg);
    ~AllocationException();
};

class HookException : public ExceptionBase {
public:
    HookException(std::string msg);
    ~HookException();
};

#endif //EXCEPTIONBASE_H
