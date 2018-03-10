#include "Connection.h"
#include "Util.h"

#include <unistd.h>
#include <strings.h>
#include <sstream>
#include <ctime>
#include <initializer_list>

using namespace util;


#define LOG_TAG "EVIL_LIB::CONNECTION"

class TimeInterval {
private:
    std::clock_t startTime;
public:
    TimeInterval() {
        startTime  = std::clock();
    }

    void start() {
        startTime  = std::clock();
    }

    long pastTime() {
        std::clock() - startTime;
    };

    double pastTimeSeconds() {
        return (double)pastTime() / (double)CLOCKS_PER_SEC;
    }
};

Connection::Connection(const char *serverName, int serverPort, int timeOutSeconds) throw(ConnectionException) {
    serverResult = NULL;
    socketFileDesc = -1;
    connected = false;
    this->timeOutSeconds =  timeOutSeconds;


    connectToServer(serverName, serverPort);
}

Connection::~Connection() {
    logE("EvilLib::Connection", "Connection::~Connection called!");
}

void Connection::disconnect() {

    logE("EvilLib::Connection", "Connection::disconnect called!");
    if (socketFileDesc != -1)
        close(socketFileDesc);
    socketFileDesc = -1;

    serverResult = NULL;
    connected = false;
}

void Connection::connectToServer(const char *serverName, int serverPort) throw(ConnectionException) {
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

    __connect(serverName, serverPort, timeOutSeconds);
}

void Connection::__connect(const char *serverName, int serverPort, int timeOutSeconds) throw(ConnectionException){

    fd_set fdset;
    struct timeval tv;
    int so_error;
    int  rc;
    TimeInterval timer;
    socklen_t len;


    setSocketBlocking(socketFileDesc, false);

    timer.start();

    rc = connect(socketFileDesc,(struct sockaddr *) &serverAddress, sizeof(sockaddr_in));



    //if (connect(socketFileDesc,(struct sockaddr *) &serverAddress, sizeof(sockaddr_in)) < 0)  {
    //    errorConnect("Couldn't connect to the server");
    //}


    if ((rc == -1) && (errno != EINPROGRESS)) {
        std::ostringstream ss;
        ss << strerror(errno);
        errorConnect(ss.str());
    }
    if (rc == 0) {
        // connection has succeeded immediately
        logE(LOG_TAG, "socket %s:%d connected. It took %.5f seconds\n",
               serverName, serverPort, timer.pastTimeSeconds());

        connected = true;
        setSocketBlocking(socketFileDesc, true);
        return;
    }

    FD_ZERO(&fdset);
    FD_SET(socketFileDesc, &fdset);
    tv.tv_sec = timeOutSeconds;
    tv.tv_usec = 0;

    rc = select(socketFileDesc + 1, NULL, &fdset, NULL, &tv);
    switch(rc) {
        case 1: // data to read
            len = sizeof(so_error);

            getsockopt(socketFileDesc, SOL_SOCKET, SO_ERROR, &so_error, &len);

            if (so_error == 0) {
                logE(LOG_TAG, "socket %s:%d connected. It took %.5f seconds\n",
                     serverName, serverPort,timer.pastTimeSeconds());
            } else { // error
                std::ostringstream ss;
                ss << "socket " << serverName << ":" << serverPort << " is not connected: " << strerror(so_error);
                errorConnect(ss.str());

            }
            break;
        case 0: //timeout
            std::ostringstream ss;
            ss << "connection timeout trying to connect to" << serverName << ":" << serverPort;
            errorConnect(ss.str());
            break;
    }

    connected = true;
    setSocketBlocking(socketFileDesc, true);
}

void Connection::errorConnect(std::string msg) throw(ConnectionException){
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

bool Connection::setSocketBlocking(int socket, bool blocking) {
    if (socket < 0) return false;

#ifdef _WIN32
    unsigned long mode = blocking ? 0 : 1;
   return (ioctlsocket(fd, FIONBIO, &mode) == 0) ? true : false;
#else
    int flags = fcntl(socket, F_GETFL, 0);
    if (flags == -1) return false;
    flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
    return (fcntl(socket, F_SETFL, flags) == 0) ? true : false;
#endif
}
