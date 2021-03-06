#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "server.hpp"
#include "error.hpp"

namespace Kahoot
{
    constexpr int QUEUE_LENGTH = 32;

    Server::Server(Config &&config)
        : m_config(std::move(config)), m_socket_fd(-1), m_epoll_fd(-1), m_clients({}), m_clients_modified(false)
    {
        m_logger = spdlog::get("console");
        m_address = {
            .sin_family = AF_INET,
            .sin_port = htons(m_config.port),
            .sin_addr = {.s_addr = htonl(INADDR_ANY)},
            .sin_zero = {},
        };
    }

    /**
     * Terminates server thread with error.
     * @param description Description of error and its origin.
     */
    void Server::terminate(const std::string &description)
    {
        m_logger->critical(description);

        // Close all opened clients' sockets.
        for (const auto &[client, _] : m_clients)
        {
            shutdown(client, SHUT_RDWR);
            close(client);
        }

        m_clients.clear();

        // Close server's listening socket.
        shutdown(m_socket_fd, SHUT_RDWR);
        close(m_socket_fd);

        // This should be handled in appropriate scope.
        // If not handled will terminate server main thread.
        throw CriticalError();
    }

    auto Server::make_socket_nonblocking(int socket_fd) -> tl::expected<void, Server::Error>
    {
        int flags = fcntl(socket_fd, F_GETFL);
        if (flags != -1)
        {
            flags |= O_NONBLOCK;
            int status = fcntl(socket_fd, F_SETFL, flags);
            if (status != -1)
            {
                return {};
            }
        }

        std::string msg = "cannot set socket as nonblocking. call to fcntl() failed. error: ";
        msg.reserve(200);

        switch (errno)
        {
        case EACCES:
            msg.append("EACCES Operation is prohibited by locks held by other processes.");
            break;

        case EAGAIN:
            msg.append("The operation is prohibited because the file has been memory-mapped by another process.");
            break;

        case EBADF:
            msg.append("EBADF Supplied descriptor is not a valid file descriptor.");
            break;

        case EFAULT:
            msg.append("EFAULT Lock is outside your accessible address space.");
            break;

        case EINVAL:
            msg.append("EINVAL The value specified in cmd is not recognized by this kernel.");
            break;

        case ENOLCK:
            msg.append("ENOLCK Too many segment locks open, lock table is full, or a remote locking protocol failed(e.g., locking over NFS)");
            break;

        default:
            msg.append("Unknown error.");
        }

        errno = 0;

        return tl::make_unexpected(Server::Error(std::move(msg)));
    }

    void Server::create_socket()
    {
        m_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (m_socket_fd == -1)
        {
            std::string msg = "cannot create socket. call to socket() failed. error: ";
            msg.reserve(200);

            switch (errno)
            {
            case EACCES:
                msg.append("EACCES Permission denied.");
                break;

            case EAFNOSUPPORT:
                msg.append("EAFNOSUPPORT Address Family not supported.");
                break;

            case EINVAL:
                msg.append("EINVAL Unknown protocol or invalid flags in type.");
                break;

            case EMFILE:
                msg.append("EMFILE Maximum number of open files for process reached.");
                break;

            case ENFILE:
                msg.append("ENFILE Maximum number of open files for operating system reached.");
                break;

            case ENOBUFS:
                [[fallthrough]];
            case ENOMEM:
                msg.append("ENOMEM Not enough memory available.");
                break;

            case EPROTONOSUPPORT:
                msg.append("EPROTONOSUPPORT Specified protocol or type is not supported in this domain.");
                break;

            default:
                msg.append("Unknown error.");
                break;
            }

            terminate(msg);
        }

        // Allow instant rebinding in case of server restart.
        const int enabled = 1;
        if (setsockopt(m_socket_fd, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(int)) == -1)
        {
            std::string msg = "cannot set socket options. call to setsockopt() failed. error: ";
            msg.reserve(200);

            switch (errno)
            {
            case EBADF:
                msg.append("EBADF Socket descriptor is invalid.");
                break;

            case EFAULT:
                msg.append("EFAULT Address of option enabling flagg is outside of process address space.");
                break;

            case EINVAL:
                msg.append("EINVAL Invalid size of enabling flag.");
                break;

            case ENOPROTOOPT:
                msg.append("ENOPROTOOPT Unknown flag at indicated level.");
                break;

            case ENOTSOCK:
                msg.append("ENOTSOCK Given descriptor does not refer to a socket.");
                break;

            default:
                msg.append("Unknown error.");
                break;
            }

            terminate(msg);
        }

        auto result = make_socket_nonblocking(m_socket_fd);
        if (!result)
        {
            terminate(result.error().description());
        }
    }

    void Server::bind_socket()
    {
        if (bind(m_socket_fd, (sockaddr *)&m_address, sizeof(sockaddr_in)) == -1)
        {
            std::string msg = "cannot bind port. call to bind() failed. error: ";
            msg.reserve(200);

            switch (errno)
            {
            case EACCES:
                msg.append("EACCES The address is protected, and the user is not the superuser.");
                break;

            case EADDRINUSE:
                msg.append("EADDRINUSE The given address is already in use.");
                break;

            case EBADF:
                msg.append("EBADF Socket descriptor is invalid.");
                break;

            case EINVAL:
                msg.append("EINVAL The socket is already bound to an address or address is not valid for socket's domain.");
                break;

            case ENOTSOCK:
                msg.append("ENOTSOCK Given descriptor does not refer to a socket.");
                break;

            default:
                msg.append("Unknown error.");
                break;
            }

            terminate(msg);
        }
    }

    void Server::listen_on_socket()
    {
        if (listen(m_socket_fd, QUEUE_LENGTH) == -1)
        {
            std::string msg = "cannot listen on given port. call to listen() failed. error: ";
            msg.reserve(200);

            switch (errno)
            {
            case EADDRINUSE:
                msg.append("EADDRINUSE Another socket is already listening on the same port.");
                break;

            case EBADF:
                msg.append("EBADF Socket descriptor is invalid.");
                break;

            case ENOTSOCK:
                msg.append("ENOTSOCK Given descriptor does not refer to a socket.");
                break;

            case EOPNOTSUPP:
                msg.append("EOPNOTSUPP Socket does not support listening on it.");
                break;

            default:
                msg.append("Unknown error.");
                break;
            }

            terminate(msg);
        }
    }

    void Server::init_epoll()
    {
        m_epoll_fd = epoll_create1(0);
        if (m_epoll_fd == -1)
        {
            std::string msg = "cannot create epoll descriptor. call to epoll_create1() failed. error: ";
            msg.reserve(200);

            switch (errno)
            {
            case EINVAL:
                msg.append("EINVAL Invalid value specified in flags.");
                break;
            case EMFILE:
                msg.append("EMFILE The user reached max number of epoll instances or open file descriptors.");
                break;

            case ENFILE:
                msg.append("ENFILE Maximum number of open files for operating system reached.");
                break;

            case ENOMEM:
                msg.append("ENOMEM Not enough memory available.");
                break;

            default:
                msg.append("Unknown error.");
                break;
            }

            terminate(msg);
        }

        m_epoll_listener = epoll_event{
            .events = EPOLLIN,
            .data{
                .fd = m_socket_fd,
            },
        };

        int epoll_ctl_status = epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_socket_fd, &m_epoll_listener);
        if (epoll_ctl_status == -1)
        {
            std::string msg = "cannot add epoll event. call to epoll_ctl() failed. error: ";
            msg.reserve(200);

            switch (errno)
            {
            case EBADF:
                msg.append("EBADF Invalid epoll descriptor.");
                break;

            case EEXIST:
                msg.append("EEXIST Descriptor already registered in epoll.");
                break;

            case EINVAL:
                msg.append("EINVAL Invalid epoll descriptor or watched descriptor is the same as epoll descriptor or given operation is invalid.");
                break;

            case ELOOP:
                msg.append("ELOOP Watched descriptor refers to another epoll instance and could result in circular dependency.");
                break;

            case ENOENT:
                msg.append("ENOENT Operation was EPOLL_CTL_MOD or EPOLL_CTL_DEL and supplied descriptor was not registered in epoll instance.");
                break;

            case ENOMEM:
                msg.append("ENOMEM Not enough memory available.");
                break;

            case ENOSPC:
                msg.append("ENOSPC Maximum number of allowed user watches reached.");
                break;

            case EPERM:
                msg.append("EPERM Supplied descriptor does not support epoll.");
                break;

            default:
                msg.append("Unknown error.");
                break;
            }

            terminate(msg);
        }
    }

    void Server::connect_clients()
    {
        // We have a notification on the listening socket, which means one **or more** incoming connections.
        while (true)
        {
            sockaddr_in in_addr;
            socklen_t in_len = sizeof(in_addr);

            in_len = sizeof in_addr;
            int connection = accept(m_socket_fd, reinterpret_cast<sockaddr *>(&in_addr), &in_len);
            if (connection == -1)
            {
                if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
                {
                    // All incoming connections have been processed.
                    break;
                }
                else
                {
                    // I don't really think I want to resolve errno myself here ??\_(???)_/??, so std::strerror comes into action.
                    m_logger->error("connection error. accept() failed with error: {}", std::strerror(errno));
                    errno = 0;
                    continue;
                }
            }

            // Make the incoming socket non-blocking and add it to the list of fds to monitor.
            auto status = make_socket_nonblocking(connection);
            if (!status)
            {
                m_logger->error(status.error().description());
                close(connection);
            }

            std::lock_guard<std::mutex> clients_guard(m_clients_mutex);
            m_clients[connection] = Client(connection, in_addr, m_config.connection_timeout);
        }
    }

    void Server::resolve_clients() {
        std::unique_lock<std::mutex> clients_guard(m_clients_mutex);
        m_clients_access.wait(clients_guard, [this]{ return m_clients_modified; });
        std::vector<int> remove_fds(m_clients.size());

        for(auto& [fd, client] : m_clients) {
            auto message = client.read_message();
            if(message.header.opcode == Opcode::CreateNewGame) {
                // Dispatch to new GameBuilder thread.
                remove_fds.push_back(fd);
            } else if(message.header.opcode == Opcode::JoinGame) {
                // Find game and dispatch to appropriate Game thread.
                remove_fds.push_back(fd);
            }
            // We ignore other messages.
        }

        for(auto fd : remove_fds) {
            m_clients.erase(fd);
        }
    }

    /**
     * Server main loop.
     */
    void Server::run()
    {

        create_socket();

        bind_socket();

        listen_on_socket();

        init_epoll();

        m_logger->info("Server is listening on ADDRESS: {} | PORT: {} | MAX CONNECTIONS: {}", inet_ntoa(m_address.sin_addr), htons(m_address.sin_port), m_config.max_connection_count);

        epoll_event socket_answer{};
        std::string msg;
        msg.reserve(256);

        // Server should work indefinitely, unless it's restarted or critical error occurs.
        while (true)
        {
            // Server can wait indefinitely, unless some connection is made.
            int epoll_status = epoll_wait(m_epoll_fd, &socket_answer, 1, -1);
            if (epoll_status == -1)
            {
                msg.clear();
                msg.append("epoll returned with error: ");

                switch (errno)
                {
                case EBADF:
                    msg.append("EBADF Invalid epoll descriptor supplied.");
                    break;

                case EFAULT:
                    msg.append("EFAULT The memory area pointed to by events is not accessible with write permissions.");
                    break;

                case EINTR:
                    msg.append("EINTR The  call  was  interrupted by a signal handler before the requested events occurred.");
                    break;

                case EINVAL:
                    msg.append("EINVAL Invalid epoll descriptor, or maxevents is less than or equal to zero.");
                    break;

                default:
                    msg.append("Unknown error.");
                    break;
                }

                m_logger->error(msg);

                continue;
            }

            // epoll can return only 1 result, as only server listening socket is watched.
            // Because we've made listening socket nonblocking, we can accept new connections in this thread
            // without blocking it undefinitely.
            connect_clients();
        }
    }
}