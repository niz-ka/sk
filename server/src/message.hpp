#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <cmath>
#include <cstring>

namespace Operation
{
    enum Type : uint32_t
    {
        NON = 0x00,
        // Keep-alive request. If ignored 3 times connection should be closed as inactive.
        REQ = 0x01,
        // Acknowledge response. Should be sent in response to REQ.
        ACK = 0x02,
        // New game creation request. Sent by client if it wants to create new quiz.
        NGA = 0x04,
        // Join game request. Sent by client if it wants to join to existing game.
        JGA = 0x08,
        // New question request. Sent by client which previously sent NGA request, to add question to quiz.
        NQN = 0x10,
        // Add answer request.  Sent by client which previously sent NQN request, to add question to quiz.
        ANS = 0x20,
        // Finish game creation request. Sent by client which previously sent NGA request, to end quiz creation phase.
        FGC = 0x40,
        // Abort game creation request. Sent by client which previously sent NGA request, to abort quiz creation phase.
        AGC = 0x80,
        // Start game request. Sent by client, which previously created quiz.
        SGA = 0xf00,
        // TODO ...
    };
}

struct MessageHeader {
    uint32_t opcode;
    uint32_t content_length;
};

struct Message
{
    MessageHeader header;
    std::vector<std::byte> content;

    template <typename R>
    R into() const;
};

// Transforms message buffer into std::string.
template <>
std::string Message::into<std::string>() const
{
    return std::string(reinterpret_cast<const char *>(content.data()), content.size());
}

// Transforms message buffer into vector of ints.
// Every 4 bytes of buffer are interpreted as 1 integer.
template <>
std::vector<int> Message::into<std::vector<int>>() const
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