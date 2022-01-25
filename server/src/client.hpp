#pragma once

#include <netinet/in.h>

#include "socket.hpp"

namespace Kahoot
{
    class Client
    {
    public:
        Client() = default;
        Client(int socketfd, sockaddr_in address, uint32_t timeout) : m_address(address), m_socket(socketfd, timeout) {}

        auto get_socket_fd() const -> int { return m_socket.fd(); }
        auto get_address() const -> const sockaddr_in * { return &m_address; }

        auto send_message(Message message) -> void { m_socket.write_message(message); }
        auto read_message() -> Message { return m_socket.read_message(); }

    private:
        sockaddr_in m_address;
        Socket m_socket;
    };
}