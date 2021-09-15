#include <fcgi_responder/request.h>
#include "msgparams.h"
#include "streamdatamessage.h"
#include <algorithm>

namespace fcgi{

namespace {

const std::string& emptyParamValue()
{
    static const auto value = std::string{};
    return value;
}

struct ParamLookupComparator{
    bool operator()(const std::string& key, const std::pair<std::string, std::string>& val) const
    {
        return key < val.first;
    }
    bool operator()(const std::pair<std::string, std::string>& val, const std::string& key) const
    {
        return val.first < key;
    }
};
}

const std::string& Request::stdIn() const
{
    return stdIn_;
}

void Request::addData(const fcgi::MsgStdIn& msg)
{
    stdIn_ += msg.data();
}

const std::string& Request::param(const std::string& name) const
{
    auto itRange = std::equal_range(params_.begin(), params_.end(), name, ParamLookupComparator{});
    if (!std::distance(itRange.first, itRange.second))
        return emptyParamValue();
    return itRange.first->second;
}

std::vector<std::string> Request::paramList() const
{
    auto result = std::vector<std::string>{};
    for (const auto& paramVal : params_)
        result.push_back(paramVal.first);
    return result;
}

bool Request::hasParam(const std::string &name) const
{
    return std::binary_search(params_.begin(), params_.end(), name, ParamLookupComparator{});
}

void Request::addParams(const fcgi::MsgParams& msg)
{
    for(const auto& paramName : msg.paramList())
        params_.emplace_back(paramName, msg.paramValue(paramName));

    postProcessParamsContainer();
}

void Request::addParams(const std::vector<std::pair<std::string, std::string>>& params)
{
    std::copy(params.begin(), params.end(), std::back_inserter(params_));
    postProcessParamsContainer();
}

void Request::postProcessParamsContainer()
{
    auto paramsEnd = std::unique(params_.begin(), params_.end(),
        [](const auto& lhsPair, const auto& rhsPair){
            return lhsPair.first == rhsPair.first;
        });
    const auto paramsSize = static_cast<std::size_t>(std::distance(params_.begin(), paramsEnd));
    params_.resize(paramsSize);
    std::sort(params_.begin(), params_.end(),
        [](const auto& lhsPair, const auto& rhsPair){
            return lhsPair.first < rhsPair.first;
        });
}

}
