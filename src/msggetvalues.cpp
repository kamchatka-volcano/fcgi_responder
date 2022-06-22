#include "msggetvalues.h"
#include "namevalue.h"
#include "errors.h"
#include <algorithm>

namespace fcgi{

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
    auto readBytes = 0u;
    while(readBytes < inputSize){
        auto nameValue = NameValue{inputSize};
        nameValue.fromStream(input);
        readBytes += nameValue.size();
        auto request = valueRequestFromString(nameValue.name());
        valueRequestList_.push_back(request);
    }
    if (readBytes != inputSize)
        throw ProtocolError{"Misaligned name-value"};
}

bool operator==(const MsgGetValues &lhs, const MsgGetValues &rhs)
{
    return lhs.valueRequestList_ == rhs.valueRequestList_;
}

}
