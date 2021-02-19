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
    sender_(std::move(data_), std::move(errorMsg_));

    //set empty sender, so response can be sent only once
    sender_ = [](std::string&&, std::string&&){};
}

}
