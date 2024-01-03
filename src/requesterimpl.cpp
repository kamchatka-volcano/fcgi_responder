#include "requesterimpl.h"
#include "record.h"
#include "recordreader.h"
#include "streammaker.h"
#include <algorithm>
#include <cstdint>
#include <exception>

namespace fcgi {

namespace {
std::set<std::uint16_t> generateRequestIds(int maxRequestsNumber)
{
    auto result = std::set<std::uint16_t>{};
    for (auto i = 1; i <= maxRequestsNumber; ++i)
        result.insert(static_cast<std::uint16_t>(i));
    return result;
}

[[noreturn]] inline void ensureNotReachable() noexcept
{
    std::terminate();
}
} //namespace

RequesterImpl::RequesterImpl(std::function<void(const std::string&)> sendData, std::function<void()> disconnect)
    : recordReader_{[this](const Record& record)
                    {
                        onRecordRead(record);
                    }}
    , recordStream_{hardcoded::maxRecordSize}
    , sendData_{std::move(sendData)}
    , disconnect_{std::move(disconnect)}
{
}

std::optional<RequestHandle> RequesterImpl::sendRequest(
        std::map<std::string, std::string> params,
        std::string data,
        const std::function<void(std::optional<ResponseData>)>& responseHandler,
        bool keepConnection)
{
    if (connectionState_ == ConnectionState::NotConnected) {
        initConnection(std::move(params), std::move(data), responseHandler, keepConnection);
        connectionOpeningRequestCancelHandler_ = std::make_shared<std::function<void()>>(
                [=]
                {
                    notifyAboutError("Connection initialization cancelled");
                    connectionState_ = ConnectionState::NotConnected;
                    responseHandler(std::nullopt);
                });
        return connectionOpeningRequestCancelHandler_;
    }
    else if (connectionState_ == ConnectionState::Connected)
        return doSendRequest(params, data, responseHandler, keepConnection);

    return std::nullopt;
}

int RequesterImpl::availableRequestsNumber() const
{
    switch (connectionState_) {
    case ConnectionState::NotConnected:
        return 1;
    case ConnectionState::ConnectionInProgress:
        return 0;
    case ConnectionState::Connected:
        return static_cast<int>(requestIdPool_.size());
    }
    ensureNotReachable();
}

void RequesterImpl::initConnection(
        std::map<std::string, std::string> params,
        std::string data,
        std::function<void(std::optional<ResponseData>)> responseHandler,
        bool keepConnection)
{
    connectionState_ = ConnectionState::ConnectionInProgress;
    onConnectionFail_ = [=]()
    {
        connectionState_ = ConnectionState::NotConnected;
        responseHandler(std::nullopt);
    };
    onConnectionSuccess_ = [this,
                            params = std::move(params),
                            data = std::move(data),
                            responseHandler = std::move(responseHandler),
                            keepConnection]() mutable
    {
        connectionState_ = ConnectionState::Connected;
        requestIdPool_ = cfg_.multiplexingEnabled ? generateRequestIds(cfg_.maxRequestsNumber) : std::set<std::uint16_t>{1};
        doSendRequest(params, data, responseHandler, keepConnection);
        *connectionOpeningRequestCancelHandler_ = *responseMap_.begin()->second.cancelRequestHandler;
    };
    auto getValuesMsg = MsgGetValues{};
    getValuesMsg.requestValue(ValueRequest::MaxReqs);
    getValuesMsg.requestValue(ValueRequest::MpxsConns);
    sendMessage(0, getValuesMsg);
}

std::optional<RequestHandle> RequesterImpl::doSendRequest(
        const std::map<std::string, std::string>& params,
        const std::string& data,
        std::function<void(std::optional<ResponseData>)> responseHandler,
        bool keepConnection)
{
    if (requestIdPool_.empty()) {
        notifyAboutError("Maximum requests number reached");
        responseHandler(std::nullopt);
        return std::nullopt;
    }

    auto requestId = *requestIdPool_.begin();
    requestIdPool_.erase(requestId);
    responseMap_.emplace(
            requestId,
            ResponseContext{
                    std::move(responseHandler),
                    ResponseData{},
                    keepConnection,
                    std::make_shared<std::function<void()>>(
                            [=]
                            {
                                doEndRequest(requestId, ResponseStatus::Cancelled);
                            })});

    sendMessage(
            requestId,
            MsgBeginRequest{
                    Role::Responder,
                    keepConnection ? ResultConnectionState::KeepOpen : ResultConnectionState::Close});
    auto paramsMsg = MsgParams{};
    for (const auto& [paramName, paramValue] : params)
        paramsMsg.setParam(paramName, paramValue);
    sendMessage(requestId, std::move(paramsMsg));
    if (!params.empty())
        sendMessage(requestId, MsgParams{});

    auto dataStream = makeStream<MsgStdIn>(requestId, data);
    std::for_each(
            dataStream.begin(),
            dataStream.end(),
            [this](const Record& record)
            {
                sendRecord(record);
            });

    return responseMap_.at(requestId).cancelRequestHandler;
}

void RequesterImpl::doEndRequest(std::uint16_t requestId, ResponseStatus responseStatus)
{
    auto& responseContext = responseMap_.at(requestId);
    if (responseStatus == ResponseStatus::Successful)
        responseContext.responseHandler(std::move(responseContext.responseData));
    else
        responseContext.responseHandler(std::nullopt);

    if (responseStatus != ResponseStatus::Cancelled && !responseContext.keepConnection)
        disconnect_();

    responseMap_.erase(requestId);
    requestIdPool_.insert(requestId);
}

template<typename TMsg>
void RequesterImpl::sendMessage(std::uint16_t requestId, TMsg&& msg)
{
    auto record = Record{std::forward<TMsg>(msg), requestId};
    sendRecord(record);
}
template void RequesterImpl::sendMessage<MsgGetValues>(std::uint16_t requestId, MsgGetValues&& msg);
template void RequesterImpl::sendMessage<MsgBeginRequest>(std::uint16_t requestId, MsgBeginRequest&& msg);
template void RequesterImpl::sendMessage<MsgAbortRequest>(std::uint16_t requestId, MsgAbortRequest&& msg);
template void RequesterImpl::sendMessage<MsgParams>(std::uint16_t requestId, MsgParams&& msg);
template void RequesterImpl::sendMessage<MsgStdIn>(std::uint16_t requestId, MsgStdIn&& msg);

void RequesterImpl::sendRecord(const Record& record)
{
    recordStream_.resetBuffer(record.size());
    try {
        record.toStream(recordStream_);
    }
    catch (std::exception& e) {
        notifyAboutError(e.what());
        return;
    }
    sendData_(recordStream_.buffer());
}

void RequesterImpl::receiveData(const char* data, std::size_t size)
{
    recordReader_.read(data, size);
}

bool RequesterImpl::isRecordExpected(const Record& record)
{
    if (record.type() == RecordType::GetValuesResult)
        return true;
    if (record.type() == RecordType::UnknownType || record.type() == RecordType::StdOut ||
        record.type() == RecordType::StdErr || record.type() == RecordType::EndRequest)
        return responseMap_.count(record.requestId());

    return false;
}

void RequesterImpl::onRecordRead(const Record& record)
{
    if (!isRecordExpected(record)) {
        notifyAboutError(
                "Received unexpected record, RecordType = " + std::to_string(static_cast<int>(record.type())) +
                ", requestId = " + std::to_string(record.requestId()));
        return;
    }

    switch (record.type()) {
    case RecordType::GetValuesResult:
        onGetValuesResult(record.getMessage<MsgGetValuesResult>());
        break;
    case RecordType::UnknownType:
        onUnknownType(record.requestId(), record.getMessage<MsgUnknownType>());
        break;
    case RecordType::EndRequest:
        onEndRequest(record.requestId(), record.getMessage<MsgEndRequest>());
        break;
    case RecordType::StdOut:
        onStdOut(record.requestId(), record.getMessage<MsgStdOut>());
        break;
    case RecordType::StdErr:
        onStdErr(record.requestId(), record.getMessage<MsgStdErr>());
        break;
    default:;
    }
}

void RequesterImpl::onGetValuesResult(const MsgGetValuesResult& msg)
{
    for (auto request : msg.requestList()) {
        switch (request) {
        case ValueRequest::MaxReqs:
            try {
                cfg_.maxRequestsNumber = std::stoi(msg.requestValue(request));
            }
            catch (std::exception&) {
                notifyAboutError("Invalid value for MaxReqs: " + msg.requestValue(request));
                onConnectionFail_();
                return;
            }
            break;
        case ValueRequest::MpxsConns:
            try {
                cfg_.multiplexingEnabled = std::stoi(msg.requestValue(request)) != 0;
            }
            catch (std::exception&) {
                notifyAboutError("Invalid value for MpxsConns: " + msg.requestValue(request));
                onConnectionFail_();
                return;
            }
            break;
        default:;
        }
    }
    onConnectionSuccess_();
}

void RequesterImpl::onUnknownType(std::uint16_t requestId, const MsgUnknownType& msg)
{
    notifyAboutError("Received unknown record type: " + std::to_string(msg.unknownTypeValue()));
    sendMessage(requestId, MsgAbortRequest{});
}

void RequesterImpl::onEndRequest(std::uint16_t requestId, const MsgEndRequest& msg)
{
    doEndRequest(
            requestId,
            msg.protocolStatus() == ProtocolStatus::RequestComplete ? ResponseStatus::Successful
                                                                    : ResponseStatus::Failed);
}

void RequesterImpl::onStdOut(std::uint16_t requestId, const MsgStdOut& msg)
{
    responseMap_.at(requestId).responseData.data += msg.data();
}

void RequesterImpl::onStdErr(std::uint16_t requestId, const MsgStdErr& msg)
{
    responseMap_.at(requestId).responseData.errorMsg += msg.data();
}

void RequesterImpl::setErrorInfoHandler(const std::function<void(const std::string&)>& handler)
{
    errorInfoHandler_ = handler;
    recordReader_.setErrorInfoHandler(errorInfoHandler_);
}

void RequesterImpl::notifyAboutError(const std::string& errorMsg)
{
    if (errorInfoHandler_)
        errorInfoHandler_(errorMsg);
}

int RequesterImpl::maximumConnectionsNumber() const
{
    return cfg_.maxConnectionsNumber;
}

int RequesterImpl::maximumRequestsNumber() const
{
    return cfg_.maxRequestsNumber;
}

bool RequesterImpl::isMultiplexingEnabled() const
{
    return cfg_.multiplexingEnabled;
}

} //namespace fcgi