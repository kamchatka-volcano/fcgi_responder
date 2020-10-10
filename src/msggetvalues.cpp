#include "msggetvalues.h"
#include "namevalue.h"
#include "errors.h"
#include <algorithm>

using namespace fcgi;

MsgGetValues::MsgGetValues()
    : Message (RecordType::GetValues)
{
}

std::size_t MsgGetValues::size() const
{
    auto result = 0u;
    for(auto request : valueRequestList_){
        result += NameValue(valueRequestToString(request), "").size();
    }
    return result;
}

void MsgGetValues::requestValue(ValueRequest request)
{
    if (std::find(valueRequestList_.begin(),
                  valueRequestList_.end(),
                  request) == valueRequestList_.end())
        valueRequestList_.push_back(request);
}

const std::vector<ValueRequest>& MsgGetValues::requestList() const
{
    return valueRequestList_;
}

void MsgGetValues::toStream(std::ostream &output) const
{
    for(auto request : valueRequestList_){
        auto nameValue = NameValue(valueRequestToString(request), "");
        nameValue.toStream(output);
    }
}

void MsgGetValues::fromStream(std::istream &input, std::size_t inputSize)
{
    auto readedBytes = 0u;
    while(true){
        auto nameValue = NameValue{};
        nameValue.fromStream(input);
        readedBytes += nameValue.size();
        auto request = valueRequestFromString(nameValue.name());
        if (request != ValueRequest::Invalid)
            valueRequestList_.push_back(request);

        if (readedBytes == inputSize)
            break;
        if (readedBytes > inputSize)
            throw MessageReadError{};
    }
}

bool MsgGetValues::operator==(const MsgGetValues& other) const
{
    return valueRequestList_ == other.valueRequestList_;
}
