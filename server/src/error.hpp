#pragma once

#include <exception>
#include <string>

namespace Kahoot
{
    class Error : public std::exception
    {
    public:
        Error() = default;
        Error(const Error &error) = default;
        Error(Error &&error) = default;
        Error(const std::string &description) : m_description(description) {}
        Error(std::string &&description) : m_description(std::move(description)) {}
        ~Error() = default;

        const char *what() const noexcept override
        {
            return m_description.c_str();
        }

    private:
        std::string m_description;
    };

    class CriticalError : public Error
    {
    public:
        CriticalError() = default;
        CriticalError(const std::string &description) : Error(description) {}
        CriticalError(std::string &&description) : Error(description) {}
        ~CriticalError() = default;
    };

    class TimeoutError : public Error
    {
    public:
        TimeoutError() = default;
        TimeoutError(const std::string &description) : Error(description) {}
        TimeoutError(std::string &&description) : Error(description) {}
        ~TimeoutError() = default;
    };

    class EpollError : public Error
    {
    public:
        EpollError() = default;
        EpollError(const std::string &description) : Error(description) {}
        EpollError(std::string &&description) : Error(description) {}
        ~EpollError() = default;
    };

}