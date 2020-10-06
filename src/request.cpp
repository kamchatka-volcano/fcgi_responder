#include "request.h"

using namespace fcgi;

const std::string& Request::stdIn() const
{
    return stdIn_;
}

std::string Request::param(const std::string& name) const
{
    auto it = params_.find(name);
    if (it == params_.end())
        return {};
    else
        return it->second;
}

std::vector<std::string> Request::paramList() const
{
    auto result = std::vector<std::string>{};
    for (const auto& paramVal : params_)
        result.push_back(paramVal.first);
    return result;
}

bool Request::keepConnection() const
{
    return keepConnection_;
}
