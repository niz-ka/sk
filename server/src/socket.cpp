#include "socket.hpp"

#include <unistd.h>

#include "error.hpp"

namespace Kahoot
{
    Socket::Socket(int socket_fd, uint32_t timeout) : m_socket_fd(socket_fd), m_timeout(timeout)
    {
        m_epoll_fd = epoll_create1(0);
        if (m_epoll_fd == -1)
        {
            throw EpollError();
        }

        m_epoll_event = epoll_event{
            .events = EPOLLIN,
            .data{
                .fd = m_socket_fd,
            },
        };

        int epoll_ctl_status = epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_socket_fd, &m_epoll_event);
        if (epoll_ctl_status == -1)
        {
            throw EpollError();
        }
    }

    Message Socket::read_message()
    {
        // Set epoll event to reading from socket
        m_epoll_event = epoll_event{
            .events = EPOLLIN,
            .data{
                .fd = m_socket_fd,
            },
        };

        int epoll_ctl_status = epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, m_socket_fd, &m_epoll_event);
        if (epoll_ctl_status == -1)
        {
            throw EpollError();
        }

        MessageHeader header;
        std::byte *header_buffer = reinterpret_cast<std::byte *>(&header);
        std::size_t index = 0;
        epoll_event event{};

        while (true)
        {
            int status = epoll_wait(m_epoll_fd, &event, 1, m_timeout);
            if (status == 0)
            {
                // TODO: What should we do after timeout?
            }
            else if (status == -1)
            {
                // Error occured
            }

            status = read(m_socket_fd, &header_buffer[index], sizeof(MessageHeader) - index);
            if (status == -1)
            {
                // Something wrong happened, couldn't read from socket, but we ignore it here.
                status = 0;
            }
            index += status;
            if (index == sizeof(MessageHeader))
            {
                index = 0;
                break;
            }
        }

        std::vector<std::byte> buffer(header.content_length);

        while (true)
        {
            int status = epoll_wait(m_epoll_fd, &event, 1, m_timeout);
            if (status == 0)
            {
                // TODO: What should we do after timeout?
            }
            else if (status == -1)
            {
                // Error occured
            }

            status = read(m_socket_fd, &buffer.data()[index], header.content_length - index);
            if (status == -1)
            {
                // Something wrong happened, couldn't read from socket, but we ignore it here.
                status = 0;
            }
            index += status;
            if (index == header.content_length)
            {
                index = 0;
                break;
            }
        }

        return Message{
            .header = header,
            .body = {
                .content = buffer,
            },
        };
    }

    void Socket::write_message(Message message)
    {
        // Maybe there is more elegant way to do this, but I haven't found it yet.
        std::vector<std::byte> buffer(sizeof(MessageHeader) + message.header.content_length);
        std::memcpy(buffer.data(), &message.header, sizeof(message.header));
        std::memcpy(buffer.data() + sizeof(message.header), message.body.content.data(), message.header.content_length);

        // Set epoll event to writing to socket
        m_epoll_event = epoll_event{
            .events = EPOLLOUT,
            .data{
                .fd = m_socket_fd,
            },
        };

        int epoll_ctl_status = epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, m_socket_fd, &m_epoll_event);
        if (epoll_ctl_status == -1)
        {
            throw EpollError();
        }

        std::size_t index = 0;
        epoll_event event{};

        while (true)
        {
            int status = epoll_wait(m_epoll_fd, &event, 1, m_timeout);
            if (status == 0)
            {
                // TODO: What should we do after timeout?
            }
            else if (status == -1)
            {
                // Error occured
            }

            status = write(m_socket_fd, reinterpret_cast<void *>(buffer.data()[index]), buffer.size() - index);
            if (status == -1)
            {
                // Something wrong happened, couldn't write to socket, but we ignore it here /for now/.
                status = 0;
            }
            index += status;
            if (index == buffer.size())
            {
                index = 0;
                break;
            }
        }
    }
}