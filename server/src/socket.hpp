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
        Message read_message();
        void write_message(Message message);

    private:
        int m_socket_fd;
        uint32_t m_timeout;

        int m_epoll_fd;
        epoll_event m_epoll_event;
    };
}
