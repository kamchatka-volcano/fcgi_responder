#include "msggetvaluesresult.h"
#include "namevalue.h"
#include "errors.h"
#include <algorithm>

namespace fcgi{

MsgGetValuesResult::MsgGetValuesResult()
    : Message(RecordType::GetValuesResult)
{
}

std::size_t MsgGetValuesResult::size() const
{
    auto result = 0u;
    for(const auto& nameValue : requestValueList_)
        result += nameValue.size();
    return result;
}

void MsgGetValuesResult::setRequestValue(ValueRequest request, const std::string& value)
{
    const auto requestStr = valueRequestToString(request);
    auto it = std::find_if(requestValueList_.begin(), requestValueList_.end(),
    [&requestStr](const NameValue& nameValue)
    {
        return nameValue.name() == requestStr;
    });

    if (it == requestValueList_.end())
        requestValueList_.emplace_back(requestStr, value);
    else
        it->setValue(value);
}

const std::string& MsgGetValuesResult::requestValue(ValueRequest request) const
{
    const auto requestStr = valueRequestToString(request);
    auto it = std::find_if(requestValueList_.begin(), requestValueList_.end(),
    [&requestStr](const NameValue& nameValue)
    {
        return nameValue.name() == requestStr;
    });

    if (it == requestValueList_.end())
        throw std::out_of_range("fcgi::MsgGetValuesResult doesn't contain value for request '" + requestStr +  "'");
    else
        return it->value();
}

std::vector<ValueRequest> MsgGetValuesResult::requestList() const
{
    auto result = std::vector<ValueRequest>{};
    for (const auto& nameValue : requestValueList_)
        result.push_back(valueRequestFromString(nameValue.name()));
    return result;
}

void MsgGetValuesResult::toStream(std::ostream &output) const
{
    for(const auto& nameValue : requestValueList_)
        nameValue.toStream(output);
}

void MsgGetValuesResult::fromStream(std::istream &input, std::size_t inputSize)
{
    auto readBytes = 0u;
    while(true){
        auto nameValue = NameValue{};
        nameValue.fromStream(input);
        readBytes += nameValue.size();
        valueRequestFromString(nameValue.name()); //Check that request name is valid
        requestValueList_.push_back(nameValue);

        if (readBytes == inputSize)
            break;
        if (readBytes > inputSize)
            throw UnrecoverableProtocolError{"Misaligned name-value"};
    }
}

bool MsgGetValuesResult::operator==(const MsgGetValuesResult& other) const
{
    return requestValueList_ == other.requestValueList_;
}

}
