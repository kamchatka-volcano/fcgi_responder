#include "response.h"

namespace fcgi{

Response::Response(ResponseSender sender)
    : sender_(sender)
{
}

Response::~Response()
{
    send();
}

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

}
