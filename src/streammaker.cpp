#include "streammaker.h"
#include "streamdatamessage.h"

namespace fcgi{

StreamMaker::StreamMaker(std::size_t maxDataMessageSize)
    : maxDataMessageSize_(maxDataMessageSize)
{
}

std::vector<Record> StreamMaker::makeStream(RecordType type, uint16_t requestId, const std::string& data)
{
    auto result = std::vector<Record>{};
    auto processMessage = [&](const std::string& msgData)
    {
        auto msg = std::make_unique<StreamDataMessage>(type, msgData);
        auto record = Record(std::move(msg), requestId);
        result.push_back(std::move(record));
    };

    auto msgData = std::string{};
    for (auto byte : data){
        msgData.push_back(byte);
        if (msgData.size() == maxDataMessageSize_){
            processMessage(msgData);
            msgData.clear();
        }

    }
    if (!msgData.empty())
        processMessage(msgData);
    processMessage("");
    return result;
}

}
