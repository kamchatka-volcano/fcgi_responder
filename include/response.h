#pragma once
#include <string>

namespace fcgi{

class Response{
public:
    Response();
    Response(const std::string& data,
             const std::string& errorMsg);
    void setData(const std::string& data);
    void setErrorMsg(const std::string& errorMsg);

    const std::string& data() const;
    const std::string& errorMsg() const;

private:
    std::string data_;
    std::string errorMsg_;
};

}
