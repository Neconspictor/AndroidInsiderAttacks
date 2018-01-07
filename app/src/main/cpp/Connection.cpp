//
// Created by Neconspictor on 02.01.2018.
//

#include "Connection.h"
#include "Util.h"

#include <unistd.h>
#include <strings.h>
#include <android/log.h>



Connection::Connection(const char *serverName, int serverPort) throw(ConnectionException) {
    serverResult = NULL;
    socketFileDesc = -1;
    connected = false;

    __connect(serverName, serverPort);
}

Connection::~Connection() {
    LOGE("Connection::~Connection called!");
}

void Connection::disconnect() {

    LOGE("Connection::disconnect called!");
    if (socketFileDesc != -1)
        close(socketFileDesc);
    socketFileDesc = -1;

    serverResult = NULL;
    connected = false;
}

void Connection::__connect(const char *serverName, int serverPort) throw(ConnectionException){

    if (connected) {
         errorConnect("Already connected!");
    }

    socketFileDesc = socket(AF_INET, SOCK_STREAM, 0);

    if (socketFileDesc < 0) {
        errorConnect("Couldn't open socket");
    }

    serverResult = gethostbyname(serverName);
    if (serverResult == NULL) {
        errorConnect("No such host");
    }

    bzero((char *) &serverAddress, sizeof(sockaddr_in));
    serverAddress.sin_family = AF_INET;

    bcopy(serverResult->h_addr,
          (char *)&(serverAddress.sin_addr.s_addr),
          serverResult->h_length);

    serverAddress.sin_port = htons(serverPort);

    if (connect(socketFileDesc,(struct sockaddr *) &serverAddress, sizeof(sockaddr_in)) < 0)  {
        errorConnect("Couldn't connect to the server");
    }

    connected = true;
}

void Connection::errorConnect(std::string&& msg) throw(ConnectionException){
    disconnect();
    throw ConnectionException(std::move(msg));
}

size_t Connection::read(void *buf, size_t maxLength) {
    return recv(socketFileDesc, buf, maxLength, 0);
}

int Connection::readInt() throw(NetworkException) {
    size_t size = sizeof(int);
    char buffer [size];
    size_t readBytes = 0;

    while(readBytes != size) {
        size_t leftBytesToRead = size - readBytes;
        size_t rawReadBytes = read(buffer + readBytes, leftBytesToRead);
        size_t currentReadBytes = get(rawReadBytes);

        if (currentReadBytes == 0) {
            throw NetworkException("Couldn't read successfully an int.");
        }

        readBytes += currentReadBytes;
    }

    int result = *((int*) buffer);

    //integers are represented in network byte order and have to be converted to host byte order
    result = ntohl(result);

    return result;
}

void Connection::write(void *buf, size_t length) throw(NetworkException){
    size_t sendBytes = send(socketFileDesc, buf, length, 0);
    if (sendBytes != length) {
        throw NetworkException("Couldn't send all bytes");
    }
}

void Connection::writeInt(int value) throw(NetworkException){
    //convert the integer first to Network byte order
    int networkByteOrder = htonl(value);

    write(&networkByteOrder, sizeof(networkByteOrder));
}

size_t Connection::get(size_t readBytes) {
    if (readBytes > 0) return readBytes;
    return 0;
}