#pragma once
#include <string>
#include <unordered_map>
#include <vector>

namespace fcgi{

class Request{
    friend class RequestEditor;

public:
    const std::string& stdIn() const;
    std::string param(const std::string& name) const;
    std::vector<std::string> paramList() const;
    bool keepConnection() const;

private:
    std::string stdIn_;
    std::unordered_map<std::string, std::string> params_;
    bool keepConnection_ = true;
};

} //namespace fcgi

