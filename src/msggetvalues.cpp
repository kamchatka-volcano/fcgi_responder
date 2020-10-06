#include "msggetvalues.h"
#include "namevalue.h"
#include <algorithm>

using namespace fcgi;

MsgGetValues::MsgGetValues()
    : Message (RecordType::GetValues)
{
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

void MsgGetValues::fromStream(std::istream &input)
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
            valueRequestList_.push_back(request);

        auto pos = input.tellg();
        if (pos == inputSize)
            break;
    }
}
