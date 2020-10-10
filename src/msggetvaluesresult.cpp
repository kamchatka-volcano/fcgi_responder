#include "msggetvaluesresult.h"
#include "namevalue.h"
#include "errors.h"

using namespace fcgi;

MsgGetValuesResult::MsgGetValuesResult()
    : Message(RecordType::GetValuesResult)
{
}

std::size_t MsgGetValuesResult::size() const
{
    auto result = 0u;
    for(const auto& requestValuePair : requestValueMap_){
        if (requestValuePair.first == ValueRequest::Invalid)
            continue;
        auto nameValue = NameValue(valueRequestToString(requestValuePair.first), requestValuePair.second);
        result += nameValue.size();
    }
    return result;
}

void MsgGetValuesResult::setRequestValue(ValueRequest request, const std::string value)
{
    requestValueMap_[request] = value;
}

std::string MsgGetValuesResult::requestValue(ValueRequest request) const
{
    auto it = requestValueMap_.find(request);
    if (it == requestValueMap_.end())
        return {};
    return it->second;
}

std::vector<ValueRequest> MsgGetValuesResult::requestList() const
{
    auto result = std::vector<ValueRequest>{};
    for (const auto& requestValuePair : requestValueMap_)
        result.push_back(requestValuePair.first);
    return result;
}

void MsgGetValuesResult::toStream(std::ostream &output) const
{
    for(const auto& requestValuePair : requestValueMap_){
        if (requestValuePair.first == ValueRequest::Invalid)
            continue;
        auto nameValue = NameValue(valueRequestToString(requestValuePair.first), requestValuePair.second);
        nameValue.toStream(output);
    }
}

void MsgGetValuesResult::fromStream(std::istream &input, std::size_t inputSize)
{
    auto readedBytes = 0u;
    while(true){
        auto nameValue = NameValue{};
        nameValue.fromStream(input);
        readedBytes += nameValue.size();
        auto request = valueRequestFromString(nameValue.name());
        if (request != ValueRequest::Invalid)
            requestValueMap_[request] = nameValue.value();

        if (readedBytes == inputSize)
            break;
        if (readedBytes > inputSize)
            throw MessageReadError{};
    }
}

bool MsgGetValuesResult::operator==(const MsgGetValuesResult& other) const
{
    return requestValueMap_ == other.requestValueMap_;
}
