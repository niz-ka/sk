#pragma once

#include <cstdint>
#include <sys/epoll.h>

#include "message.hpp"

namespace Kahoot
{
    class Socket final
    {
    public:
        Socket(int socket_fd, uint32_t timeout);
        auto fd() const -> int;
        
        auto read_message() -> Message;
        auto write_message(Message message) -> void;

    private:
        int m_socket_fd;
        uint32_t m_timeout;

        int m_epoll_fd;
        epoll_event m_epoll_event;
    };
}
