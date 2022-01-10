// #include "Server.h"

#include <iostream>

#include "config.hpp"
int main()
{
    Config config = Config::from("config.toml");

    std::cout << "Config\n\tport: " << config.port << "\n\tmax connection count: " << config.max_connection_count << "\n\tconnection timeout: " << config.connection_timeout << std::endl;
    // Server server;
    // server.run();
    return 0;
}
