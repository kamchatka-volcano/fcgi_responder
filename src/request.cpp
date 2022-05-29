#include <fcgi_responder/request.h>
#include <algorithm>

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

void sortPairList(std::vector<std::pair<std::string, std::string>>& params)
{
    auto paramsEnd = std::unique(params.begin(), params.end(),
                                 [](const auto& lhsPair, const auto& rhsPair){
                                     return lhsPair.first == rhsPair.first;
                                 });
    const auto paramsSize = static_cast<std::size_t>(std::distance(params.begin(), paramsEnd));
    params.resize(paramsSize);
    std::sort(params.begin(), params.end(),
              [](const auto& lhsPair, const auto& rhsPair){
                  return lhsPair.first < rhsPair.first;
              });
}

}

namespace fcgi{

Request::Request(std::vector<std::pair<std::string, std::string>> params, std::string stdIn)
    : params_(std::move(params))
    , stdIn_(std::move(stdIn))
{
    sortPairList(params_);
}

const std::string& Request::stdIn() const
{
    return stdIn_;
}

const std::string& Request::param(const std::string& name) const
{
    auto itRange = std::equal_range(params_.begin(), params_.end(), name, ParamLookupComparator{});
    if (!std::distance(itRange.first, itRange.second))
        return emptyParamValue();
    return itRange.first->second;
}

const std::vector<std::pair<std::string, std::string>>& Request::params() const
{
    return params_;
}

std::vector<std::string> Request::paramList() const
{
    auto result = std::vector<std::string>{};
    std::transform(params_.begin(), params_.end(),
                   std::back_inserter(result),
                   [](const auto& paramPair){return paramPair.first;});
    return result;
}

bool Request::hasParam(const std::string &name) const
{
    return std::binary_search(params_.begin(), params_.end(), name, ParamLookupComparator{});
}

}
