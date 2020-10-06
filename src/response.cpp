#include "response.h"

using namespace fcgi;

Response::Response()
{
}

Response::Response(const std::string& data, const std::string& errorMsg)
    : data_(data)
    , errorMsg_(errorMsg)
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

