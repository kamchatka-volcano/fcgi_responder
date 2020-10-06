#include "msgparams.h"
#include <algorithm>

using namespace fcgi;

MsgParams::MsgParams()
    : Message(RecordType::Params)
{
}

void MsgParams::setParam(const std::string &name, const std::string &value)
{
    auto it = std::find_if(paramList_.begin(), paramList_.end(),
    [&name](const NameValue& nameValue)
    {
        return nameValue.name() == name;
    });

    if (it == paramList_.end())
        paramList_.push_back({name, value});
    else {
        it->setValue(value);
    }
}

std::string MsgParams::paramValue(const std::string& name) const
{
    auto it = std::find_if(paramList_.begin(), paramList_.end(),
    [&name](const NameValue& nameValue)
    {
        return nameValue.name() == name;
    });

    if (it == paramList_.end())
        return {};
    else
        return it->value();
}

std::vector<std::string> MsgParams::paramList() const
{
    auto result = std::vector<std::string>{};
    for (const auto& param : paramList_)
        result.push_back(param.name());
    return result;
}

void MsgParams::toStream(std::ostream &output) const
{
    for(const auto& param : paramList_)
        param.toStream(output);
}

void MsgParams::fromStream(std::istream &input)
{
    input.seekg(0, input.end);
    auto inputSize = input.tellg();
    if (inputSize == 0)
        return;
    input.seekg(0, input.beg);

    while(true){
        auto param = NameValue{};
        param.fromStream(input);
        paramList_.push_back(param);

        auto pos = input.tellg();
        if (pos == inputSize)
            break;
    }
}

