#include <fcgi_responder/requester.h>
#include "requesterimpl.h"

namespace fcgi{

RequestHandle::RequestHandle(const std::shared_ptr<std::function<void()>>& cancelRequestHandler)
    : cancelRequestHandler_{cancelRequestHandler}
{
}

void RequestHandle::cancelRequest()
{
    if (auto cancelRequestHandler = cancelRequestHandler_.lock())
        (*cancelRequestHandler)();
}

Requester::Requester()
    : impl_{std::make_unique<RequesterImpl>(
            [this](const std::string& data){
                sendData(data);
            },
            [this](){
                disconnect();
            })}
{
}

Requester::~Requester() = default;

RequesterImpl& Requester::impl()
{
    return *impl_;
}

const RequesterImpl& Requester::impl() const
{
    return *impl_;
}

std::optional<RequestHandle> Requester::sendRequest(
        std::map<std::string, std::string> params,
        std::string data,
        const std::function<void(std::optional<ResponseData>)>& responseHandler,
        bool keepConnection)
{
    return impl().sendRequest(std::move(params), std::move(data), responseHandler, keepConnection);
}

int Requester::availableRequestsNumber() const
{
    return impl().availableRequestsNumber();
}

void Requester::receiveData(const char* data, std::size_t size)
{
    impl().receiveData(data, size);
}

void Requester::setErrorInfoHandler(const std::function<void (const std::string &)>& handler)
{
    impl().setErrorInfoHandler(handler);
}

int Requester::maximumConnectionsNumber() const
{
    return impl().maximumConnectionsNumber();
}

int Requester::maximumRequestsNumber() const
{
    return impl().maximumRequestsNumber();
}

bool Requester::isMultiplexingEnabled() const
{
    return impl().isMultiplexingEnabled();
}

}