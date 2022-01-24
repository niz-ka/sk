#include "config.hpp"
#include "toml.hpp"

namespace Kahoot
{
    Config::Config(uint32_t port, uint32_t max_connections, int32_t timeout)
        : port(port), max_connection_count(max_connections), connection_timeout(timeout)
    {
    }

    auto Config::from(const std::string &filename) -> Config
    {
        auto config = toml::parse(filename);

        auto port = toml::find_or<uint32_t>(config, "server", "port", 8080);
        auto connection_count = toml::find_or(config, "connections", "max_count", 100);
        auto connection_timeout = toml::find_or(config, "connections", "timeout", 10000);

        return Config(port, connection_count, connection_timeout);
    }
}