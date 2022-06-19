#include "msgparams.h"
#include "errors.h"
#include <algorithm>

namespace fcgi{

MsgParams::MsgParams()
    : Message(RecordType::Params)
{
}

std::size_t MsgParams::size() const
{
    auto result = 0u;
    for(const auto& param : paramList_)
        result += param.size();
    return result;
}

void MsgParams::setParam(const std::string &name, const std::string &value)
{
    auto it = std::find_if(paramList_.begin(), paramList_.end(),
    [&name](const NameValue& nameValue)
    {
        return nameValue.name() == name;
    });

    if (it == paramList_.end())
        paramList_.emplace_back(name, value);
    else
        it->setValue(value);
}

const std::string& MsgParams::paramValue(const std::string& name) const
{
    auto it = std::find_if(paramList_.begin(), paramList_.end(),
    [&name](const NameValue& nameValue)
    {
        return nameValue.name() == name;
    });

    if (it == paramList_.end())
        throw std::out_of_range("fcgi::MsgParams doesn't contain param '" + name +  "'");
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

void MsgParams::fromStream(std::istream &input, std::size_t inputSize)
{
    auto readBytes = 0u;
    while(readBytes < inputSize){
        auto param = NameValue{inputSize};
        param.fromStream(input);
        readBytes += param.size();
        paramList_.push_back(param);
    }
    if (readBytes != inputSize)
        throw ProtocolError{"Misaligned name-value"};
}

bool MsgParams::operator==(const MsgParams& other) const
{
    return paramList_ == other.paramList_;
}

}
