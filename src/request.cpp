#include "request.h"

using namespace fcgi;

const std::string& Request::stdIn() const
{
    return stdIn_;
}

std::string Request::param(const std::string& name) const
{
    return params_.at(name);
}

std::vector<std::string> Request::paramList() const
{
    auto result = std::vector<std::string>{};
    for (const auto& paramVal : params_)
        result.push_back(paramVal.first);
    return result;
}

const std::unordered_map<std::string, std::string>& Request::params() const
{
    return params_;
}

bool Request::keepConnection() const
{
    return keepConnection_;
}
