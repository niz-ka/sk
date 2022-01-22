#pragma once

#include <array>
#include <unordered_map>
#include <string>
#include <vector>
#include <future>
#include <mutex>

#include <netinet/in.h>
#include <sys/epoll.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreorder"
#include <spdlog/spdlog.h>
#include <expected.hpp>
#pragma GCC diagnostic pop

#include "client.hpp"
#include "config.hpp"
#include "game.hpp"

class Server
{
public:
    Server() = delete;
    Server(Config &&config);
    void run();

private:
    class Error
    {
        std::string m_description;

    public:
        Error() = delete;
        Error(const std::string &description) : m_description(description) {}
        Error(std::string &&description) : m_description(std::move(description)) {}

        const std::string &description() { return m_description; }
    };

private:
    void terminate(const std::string &description);
    void create_socket();
    void bind_socket();
    void listen_on_socket();
    void init_epoll();

    void connect_client();
    static tl::expected<void, Error> make_socket_nonblocking(int socket_fd);

    void incoming_connections_cleanup();
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

    int m_epoll_fd;
    epoll_event m_epoll_listener;
    epoll_event m_epoll_event;

    std::vector<std::future<void>> m_incoming_connections;

    std::unordered_map<int, Client> m_clients;
    std::vector<pollfd> m_pollfds;
    std::mutex m_connection_mutex;
};