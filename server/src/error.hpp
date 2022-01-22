#pragma once

#include <exception>

class CriticalError : public std::exception
{
public:
    CriticalError() = default;
    ~CriticalError() = default;
};
