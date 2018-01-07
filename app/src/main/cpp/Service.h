//
// Created by necon on 07.01.2018.
//

#ifndef REPO_SERVICE_H
#define REPO_SERVICE_H


#include <string>


struct Service {
    std::string serverHostName;
    unsigned int port;

    Service(std::string serverHostName, unsigned int port) {
        this->serverHostName = std::move(serverHostName);
        this->port = port;
    }

    /**
     * Copy constructor.
     * @param other The service to be copied.
     */
    Service(const Service& other) {
        this->serverHostName = other.serverHostName;
        this->port = other.port;
    }

    /**
     * Move constructor.
     * @param other The service to be copied.
     */
    Service(Service&& other) {
        this->serverHostName = std::move(other.serverHostName);
        this->port = other.port;
    }

    // Copy assignment operator
    Service& operator=(const Service& other) {
        if (this != &other) {
            serverHostName = other.serverHostName;
            port = other.port;
        }
        return *this;
    }

    //C++11 move assignment operator
    Service& operator=(Service&& other) {
        if (this != &other) {
            serverHostName = std::move(other.serverHostName);
            port = other.port;
        }

        return *this;
    }
};


#endif //REPO_SERVICE_H
