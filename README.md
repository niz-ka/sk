# SK - Kahoot

## Structure

Application is written in two parts: client and server.

### Client

Written in Java, using `java.io` and Java Swing for GUI.


### Server

Written in C++ using Linux's implementation of BSD sockets API.
Single platform, requires Linux kernel >= 2.6 (usage of epoll).

Other requirements:
    - C++17-compliant compiler (GCC 7+ or Clang 5+)
    - libpthread support
    - Cmake

## Installation

```bash
git clone --recurse-submodules https://github.com/niz-ka/sk.git kahoot
cd kahoot
```
or if you already cloned repo:

```bash
cd <repo-dir>
git pull
git submodule update --init --recursive
```

### Client

### Server

```bash
cd server
cmake -S . -B build
cmake --build build
./build/kahoot-server
```

or if you already cloned repo:

```bash
cd server
cmake -S . -B build
cmake --build build
./build/kahoot-server
```

This will run server with configuration defined in `config.toml` (from current working directory).

