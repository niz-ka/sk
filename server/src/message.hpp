#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <cmath>
#include <cstring>

namespace Kahoot
{
    enum class Opcode : uint32_t
    {
        None = 0x00,
        CreateNewGame = 0x01,
        AddQuestion = 0x02,
        AddAnswer = 0x03,
        FinishQuestion = 0x04,
        FinishGameCreation = 0x05,
        AbortGameCreation = 0x06,
        // TODO ...
    };

    struct MessageHeader
    {
        Opcode opcode;
        uint32_t content_length;
    };

    struct MessageBody
    {
        std::vector<std::byte> content;

        template <typename R>
        R into() const;
    };

    struct Message
    {
        MessageHeader header;
        MessageBody body;
    };

    // Transforms message buffer into std::string.
    template <>
    std::string MessageBody::into<std::string>() const
    {
        return std::string(reinterpret_cast<const char *>(content.data()), content.size());
    }

    // Transforms message buffer into vector of ints.
    // Every 4 bytes of buffer are interpreted as 1 integer.
    template <>
    std::vector<int> MessageBody::into<std::vector<int>>() const
    {
        // If size of target vector is ⌈sizeof(content) / sizeof(int)⌉. Ceil is required, because sizeof(content) doesn't have to be multiple of sizeof(int).
        const int new_size = std::ceil(static_cast<float>(content.size()) / sizeof(int));

        // Unfortunately I didn't found better way to reinterpret vector of bytes into vector if ints :(.
        int *ints = new int[new_size]{};
        std::memcpy(reinterpret_cast<void *>(ints), reinterpret_cast<const void *>(content.data()), content.size());

        std::vector<int> integers(new_size);
        for (int i = 0; i < new_size; ++i)
        {
            integers.push_back(ints[i]);
        }

        delete[] ints;

        return integers;

        // Alternatively we can try something like:
        // ```
        // std::vector<int> integers(new_size, 0);
        // std::memcpy(const_cast<void *>(reinterpret_cast<const void *>(integers.data())), reinterpret_cast<const void *>(content.data()), content.size());
        // return integers;
        // ```
        // No additional allocations needed, but it is highly dangerous and according to C++ spec will result in UB (modifying a const object through a non-const access path).
    }
}