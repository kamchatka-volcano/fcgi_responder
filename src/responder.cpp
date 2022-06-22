#include <fcgi_responder/responder.h>
#include "responderimpl.h"
#include <algorithm>

namespace fcgi{

Responder::Responder()
        : impl_{std::make_unique<ResponderImpl>([this](const std::string& data) { sendData(data); },
                                                [this]() { disconnect(); },
                                                [this](Request&& request, Response&& response) {
                                                    processRequest(std::move(request), std::move(response));
                                                })}
{
}

Responder::~Responder() = default;

ResponderImpl& Responder::impl()
{
    return *impl_;
}

const ResponderImpl& Responder::impl() const
{
    return *impl_;
}

void Responder::receiveData(const char* data, std::size_t size)
{
    impl().receiveData(data, size);
}

void Responder::setMaximumConnectionsNumber(int value)
{
    impl().setMaximumConnectionsNumber(value);
}

void Responder::setMaximumRequestsNumber(int value)
{
    impl().setMaximumRequestsNumber(value);
}

void Responder::setMultiplexingEnabled(bool state)
{
    impl().setMultiplexingEnabled(state);
}

void Responder::setErrorInfoHandler(std::function<void (const std::string &)> handler)
{
    impl().setErrorInfoHandler(std::move(handler));
}

int Responder::maximumConnectionsNumber() const
{
    return impl().maximumConnectionsNumber();
}

int Responder::maximumRequestsNumber() const
{
    return impl().maximumRequestsNumber();
}

bool Responder::isMultiplexingEnabled() const
{
    return impl().isMultiplexingEnabled();
}

}
