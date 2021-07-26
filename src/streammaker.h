#pragma once
#include "record.h"
#include "types.h"
#include "constants.h"
#include <vector>

namespace fcgi{

class StreamMaker{
public:
    explicit StreamMaker(std::size_t maxDataMessageSize = cMaxDataMessageSize);
    template <typename TMsg>
    std::vector<Record> makeStream(uint16_t requestId, std::string&& data)
    {
        auto result = std::vector<Record>{};
        auto processMessage = [&](std::string&& msgData)
        {
            auto msg = TMsg{std::move(msgData)};
            result.emplace_back(msg, requestId);
        };
        if (data.size() <= maxDataMessageSize_){
            if (!data.empty())
                processMessage(std::move(data));
            processMessage("");
            return result;
        }

        const auto packetsNum = data.size() / maxDataMessageSize_;
        const auto lastPacketSize = data.size() % maxDataMessageSize_;
        for (auto i = 0u; i < packetsNum; ++i){
            auto packetOffset = &data[i * maxDataMessageSize_];
            processMessage(std::string{packetOffset, packetOffset + maxDataMessageSize_});
        }
        if (lastPacketSize){
            auto packetOffset = &data[packetsNum * maxDataMessageSize_];
            processMessage(std::string{packetOffset, packetOffset + lastPacketSize});
        }
        processMessage("");
        return result;
    }

private:
    std::size_t maxDataMessageSize_;
};

}

