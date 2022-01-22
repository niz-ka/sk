#pragma once

#include <netinet/in.h>
#include <sys/poll.h>

class Client
{
public:
    Client() = default;
    constexpr Client(int socketfd, sockaddr_in address) : m_socket_fd(socketfd), m_address(address) {}

    constexpr int get_socket_fd() const { return m_socket_fd; }
    const sockaddr_in *get_address() const { return &m_address; }

private:
    int m_socket_fd;
    sockaddr_in m_address;
};