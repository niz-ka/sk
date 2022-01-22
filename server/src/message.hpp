#pragma once

#include <cstdint>
#include <vector>

enum class Operation {
    None = 0x00,
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
    // Finish game creation request. Sent by client which previously sent NGA request, to end quiz creation phase.
    FGC = 0x20,
    // Abort game creation request. Sent by client which previously sent NGA request, to abort quiz creation phase.
    AGC = 0x30,
    // Start game request. Sent by client, which previously created quiz.
    SGA = 0x40,
    // TODO ...
};

struct Message
{
    Operation op;
    uint16_t content_length;
    std::vector<uint8_t> content;
};
