//
// Created by Neconspictor on 28.12.2017.
//

#ifndef BACHELOR_SEMINAR_CONNECTION_H
#define BACHELOR_SEMINAR_CONNECTION_H


#include <netdb.h>
#include <string>
#include <netinet/in.h>
#include "Exceptions.h"

class Connection {

public:
    Connection(const char *serverName, int serverPort, int timeOutSeconds) throw(ConnectionException);

    ~Connection();

    void disconnect();

    //ssize_t recv(int __fd, void* __buf, size_t __n, int __flags)
    size_t read(void* buf, size_t maxLength);

    int readInt() throw(NetworkException);

    void write(void* buf, size_t length) throw(NetworkException);

    void writeInt(int value) throw(NetworkException);

private:
    hostent* serverResult;
    int socketFileDesc;
    sockaddr_in serverAddress;
    bool connected;
    int timeOutSeconds;


    void connectToServer(const char* serverName, int serverPort) throw(ConnectionException);
    void __connect(const char* serverName, int serverPort, int timeOutSeconds) throw(ConnectionException);


    void errorConnect(std::string msg) throw(ConnectionException);

    size_t get(size_t readBytes);

    bool setSocketBlocking(int socket, bool blocking);
};

#endif //BACHELOR_SEMINAR_CONNECTION_H