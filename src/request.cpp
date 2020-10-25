#include "request.h"

using namespace fcgi;

const std::string& Request::stdIn() const
{
    return stdIn_;
}

const std::string& Request::param(const std::string& name) const
{
    try{
        return params_.at(name);
    }
    catch(const std::out_of_range&){
        throw std::out_of_range("fcgi::Request doesn't contain param '" + name + "'");
    }
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

bool Request::hasParam(const std::string &name) const
{
    return params_.find(name) != params_.end();
}

bool Request::keepConnection() const
{
    return keepConnection_;
}
