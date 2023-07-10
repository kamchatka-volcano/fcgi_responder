#include "requestdata.h"
#include "msgparams.h"
#include <fcgi_responder/request.h>

namespace fcgi {

RequestData::RequestData(bool keepConnection)
    : keepConnection_{keepConnection}
{
}

void RequestData::addMessage(const MsgParams& msg)
{
    for (const auto& paramName : msg.paramList())
        params_.emplace_back(paramName, msg.paramValue(paramName));
}

void RequestData::addMessage(const MsgStdIn& msg)
{
    stdIn_ += msg.data();
}

std::optional<Request> RequestData::makeRequest()
{
    if (usedInRequest_)
        return std::nullopt;
    usedInRequest_ = true;

    return Request{std::move(params_), std::move(stdIn_)};
}

bool RequestData::keepConnection() const
{
    return keepConnection_;
}

} //namespace fcgi
