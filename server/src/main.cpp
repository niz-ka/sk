
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreorder"
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#pragma GCC diagnostic pop

#include "config.hpp"
#include "server.hpp"

int main()
{
    auto logger = spdlog::stdout_color_mt<spdlog::async_factory>("console");

    Config config = Config::from("config.toml");

    Server server(std::move(config));
    server.run();

    return 0;
}
