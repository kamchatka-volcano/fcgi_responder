#pragma once
#include "record.h"
#include "types.h"
#include "constants.h"
#include <vector>
#include <string_view>

namespace fcgi{

template <typename TMsg>
std::vector<Record> makeStream(uint16_t requestId, std::string_view data, std::size_t maxDataMessageSize = cMaxDataMessageSize)
{
    auto result = std::vector<Record>{};
    auto processMessage = [&](std::string_view msgData) {
        auto msg = TMsg{msgData};
        result.emplace_back(msg, requestId);
    };
    if (data.size() <= maxDataMessageSize) {
        if (!data.empty())
            processMessage(data);
        processMessage("");
        return result;
    }

    const auto packetsNum = data.size() / maxDataMessageSize;
    const auto lastPacketSize = data.size() % maxDataMessageSize;
    for (auto i = 0u; i < packetsNum; ++i) {
        auto packetOffset = &data[i * maxDataMessageSize];
        processMessage(std::string_view{packetOffset, maxDataMessageSize});
    }
    if (lastPacketSize) {
        auto packetOffset = &data[packetsNum * maxDataMessageSize];
        processMessage(std::string_view{packetOffset, lastPacketSize});
    }
    processMessage("");
    return result;
}

}

