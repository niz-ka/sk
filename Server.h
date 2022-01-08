//
// Created by kamil on 08.01.2022.
//

#ifndef SIECI_PROJEKT_SERVER_H
#define SIECI_PROJEKT_SERVER_H


#include <netinet/in.h>
#include <unordered_map>
#include <string>
#include <sys/poll.h>
#include "Client.h"

class Server {
    int socketFd;
    sockaddr_in address;
    std::unordered_map<int, Client> clients;

public:
    Server();
    void run();

private:
    void terminate(const std::string& description);
    size_t getNumberOfClients() const;
    void disconnectClient(int clientFd);
    int connectClient();
    size_t readData(int clientFd);
};


#endif //SIECI_PROJEKT_SERVER_H
