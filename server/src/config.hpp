#pragma once

#include <cstdint>
#include <string>

// Contains server startup configuration.
struct Config {
    // Port for listening for connections.
    uint32_t port;
    // Maximum number of parallel connections.
    uint32_t max_connection_count;
    // Time after which connection is closed (in milliseconds).
    // To keep connection alive client has to send message to server
    // at least once per this time span.
    // Any negative value means infinite timeout.
    int32_t connection_timeout;

public:
    Config(uint32_t port = 8080, uint32_t max_connections = 100, int32_t timeout = 10000);
    static auto from(const std::string& filename) -> Config;
};