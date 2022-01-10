//
// Created by kamil on 08.01.2022.
//

#ifndef SIECI_PROJEKT_CLIENT_H
#define SIECI_PROJEKT_CLIENT_H

#include <netinet/in.h>
#include <sys/poll.h>

class Client {
    int socketFd;
    sockaddr_in address;
public:
    Client();
    Client(int socketFd, sockaddr_in address);
    int getSocketFd() const;
    sockaddr_in* getAddressPointer();
};


#endif //SIECI_PROJEKT_CLIENT_H
