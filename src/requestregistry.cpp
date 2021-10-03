#include "requestregistry.h"
#include <fcgi_responder/request.h>
#include "msgparams.h"
#include "streamdatamessage.h"
#include "errors.h"
#include <algorithm>

namespace fcgi{

void RequestRegistry::fillRequestData(uint16_t requestId, const fcgi::MsgParams& msg)
{
    for(const auto& paramName : msg.paramList())
        requestDataMap_[requestId].params.emplace_back(paramName, msg.paramValue(paramName));
}

void RequestRegistry::fillRequestData(uint16_t requestId, const fcgi::MsgStdIn& msg)
{
    requestDataMap_[requestId].stdIn += msg.data();
}

bool RequestRegistry::hasRequest(uint16_t requestId) const
{
    return requestDataMap_.find(requestId) != requestDataMap_.end();
}

bool RequestRegistry::isEmpty() const
{
    return requestDataMap_.empty();
}

int RequestRegistry::requestCount() const
{
    return static_cast<int>(requestDataMap_.size());
}

Request RequestRegistry::makeRequest(uint16_t requestId)
{
    auto& requestData = requestDataMap_.at(requestId);
    if (requestData.movedToRequest)
        throw ProtocolError{"Trying to create request more than once"};

    requestData.movedToRequest = true;
    return Request{std::move(requestData.params), std::move(requestData.stdIn)};
}

void RequestRegistry::registerRequest(uint16_t requestId)
{
    requestDataMap_.emplace(requestId, RequestData{});
}

void RequestRegistry::unregisterRequest(uint16_t requestId)
{
    requestDataMap_.erase(requestId);
}

}