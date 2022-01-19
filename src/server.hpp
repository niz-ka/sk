#pragma once

#include <unordered_map>
#include <string>
#include <vector>

#include <netinet/in.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreorder"
#include <spdlog/spdlog.h>
#pragma GCC diagnostic pop

#include "client.hpp"
#include "config.hpp"

class Server
{
public:
    Server(Config &&config);
    void run();

private:
    void terminate(const std::string &description);
    // size_t getNumberOfClients() const;
    // void disconnectClient(int clientFd);
    // int connectClient();
    // static size_t readData(int clientFd, int length, std::string &data);
    // static int stringToInt(const std::string &number);
    // void makeAction(const std::string &message);

private:
    std::shared_ptr<spdlog::logger> m_logger;

    Config m_config;
    int m_socket_fd;
    sockaddr_in m_address;
    std::unordered_map<int, Client> m_clients;
    std::vector<pollfd> m_pollfds;
};