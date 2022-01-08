//
// Created by kamil on 08.01.2022.
//

#include "Client.h"

Client::Client() : socketFd(0), address({}) {}

Client::Client(int socketFd, sockaddr_in address)
: socketFd(socketFd), address(address) {}

int Client::getSocketFd() const {
    return this->socketFd;
}

sockaddr_in* Client::getAddressPointer() {
    return &(this->address);
}