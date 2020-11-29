#include "response.h"

namespace fcgi{

Response::Response()
{
}

Response::Response(const std::string& data, const std::string& errorMsg)
    : data_(data)
    , errorMsg_(errorMsg)
{
}

Response::Response(std::string&& data, std::string&& errorMsg)
    : data_(std::move(data))
    , errorMsg_(std::move(errorMsg))
{
}

void Response::setData(const std::string& data)
{
    data_ = data;
}

void Response::setErrorMsg(const std::string& errorMsg)
{
    errorMsg_ = errorMsg;
}

const std::string& Response::data() const
{
    return data_;
}

const std::string& Response::errorMsg() const
{
    return errorMsg_;
}

}
