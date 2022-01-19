cmake_minimum_required(VERSION 3.15...3.22)

if(${CMAKE_VERSION} VERSION_LESS 3.17)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

include(CheckIPOSupported)
check_ipo_supported(RESULT LTO_SUPPORT)

project(Kahoot VERSION 0.1.20220106 LANGUAGES CXX)

add_executable(kahoot-server src/main.cpp src/client.hpp src/client.cpp src/server.hpp src/server.cpp src/config.hpp src/config.cpp)

set_target_properties(kahoot-server PROPERTIES CXX_EXTENSIONS OFF)
target_compile_features(kahoot-server PUBLIC cxx_std_20)
target_compile_options(kahoot-server PRIVATE -Wall -Wextra -Wpedantic)
if(LTO_SUPPORT)
    set_target_properties(kahoot-server PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON)
endif()

target_include_directories(kahoot-server PRIVATE vendor/toml vendor/spdlog/include vendor/doctest)
target_link_libraries(kahoot-server PRIVATE pthread)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
