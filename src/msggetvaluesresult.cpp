#include "msggetvaluesresult.h"
#include "namevalue.h"

using namespace fcgi;

MsgGetValuesResult::MsgGetValuesResult()
    : Message(RecordType::GetValuesResult)
{
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

void MsgGetValuesResult::fromStream(std::istream &input)
{
    input.seekg(0, input.end);
    auto inputSize = input.tellg();
    if (inputSize == 0)
        return;
    input.seekg(0, input.beg);

    while(true){
        auto nameValue = NameValue{};
        nameValue.fromStream(input);
        auto request = valueRequestFromString(nameValue.name());
        if (request != ValueRequest::Invalid)
            requestValueMap_[request] = nameValue.value();

        auto pos = input.tellg();
        if (pos == inputSize)
            break;
    }
}


