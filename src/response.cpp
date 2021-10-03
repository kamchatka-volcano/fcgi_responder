#include <fcgi_responder/response.h>

namespace fcgi{

Response::Response(ResponseSender sender)
    : sender_(std::move(sender))
{
}

Response::~Response()
{
    send();
}

Response::Response(Response&&) noexcept = default;
Response& Response::operator=(Response&&) noexcept = default;

void Response::send()
{
    if (!sender_)
        return;

    sender_(std::move(data_), std::move(errorMsg_));

    //set empty sender, so response can be sent only once
    sender_ = ResponseSender{};
}

bool Response::isValid() const
{
    return sender_.operator bool();
}

Response::operator bool() const
{
    return isValid();
}

void Response::setData(std::string data)
{
    data_ = std::move(data);
}

void Response::setErrorMsg(std::string errorMsg)
{
    errorMsg_ = std::move(errorMsg);
}

}
