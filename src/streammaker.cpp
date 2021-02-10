#include "streammaker.h"
#include "streamdatamessage.h"

namespace fcgi{

StreamMaker::StreamMaker(std::size_t maxDataMessageSize)
    : maxDataMessageSize_(maxDataMessageSize)
{
}

std::vector<Record> StreamMaker::makeStream(RecordType type, uint16_t requestId, std::string&& data)
{
    auto result = std::vector<Record>{};
    auto processMessage = [&](std::string&& msgData)
    {
        auto msg = std::make_unique<StreamDataMessage>(type, std::move(msgData));
        result.emplace_back(std::move(msg), requestId);
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

}
