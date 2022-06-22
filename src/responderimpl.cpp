#include "responderimpl.h"
#include <fcgi_responder/request.h>
#include <fcgi_responder/response.h>
#include "record.h"
#include "msgbeginrequest.h"
#include "msgendrequest.h"
#include "msggetvalues.h"
#include "msggetvaluesresult.h"
#include "msgunknowntype.h"
#include "msgparams.h"
#include "streamdatamessage.h"
#include "types.h"
#include "errors.h"
#include "streammaker.h"
#include "constants.h"
#include <algorithm>

namespace fcgi{

ResponderImpl::ResponderImpl(
        std::function<void(const std::string&)> sendData,
        std::function<void()> disconnect,
        std::function<void(Request&& request, Response&& response)> processRequest)
    : recordReader_{
            [this](const Record& record){ onRecordRead(record);},
            [this](uint8_t recordType){ sendMessage(0, MsgUnknownType{recordType});}}
    , sendData_(std::move(sendData))
    , disconnect_(std::move(disconnect))
    , processRequest_(std::move(processRequest))
{
    recordBuffer_.resize(cMaxRecordSize);
    recordStream_.rdbuf()->pubsetbuf(&recordBuffer_[0], cMaxRecordSize);
}

template <typename TMsg>
void ResponderImpl::sendMessage(uint16_t requestId, TMsg&& msg)
{
    auto record = Record{std::forward<TMsg>(msg), requestId};
    sendRecord(record);
}
template void ResponderImpl::sendMessage<MsgUnknownType>(uint16_t requestId, MsgUnknownType&& msg);
template void ResponderImpl::sendMessage<MsgEndRequest>(uint16_t requestId, MsgEndRequest&& msg);
template void ResponderImpl::sendMessage<MsgGetValuesResult>(uint16_t requestId, MsgGetValuesResult&& msg);


void ResponderImpl::receiveData(const char* data, std::size_t size)
{
    recordReader_.read(data, size);
}

void ResponderImpl::onRecordRead(const Record& record)
{
    if (!isRecordExpected(record)){
        notifyAboutError("Received unexpected record, RecordType = "
                         + std::to_string(static_cast<int>(record.type()))
                         + ", requestId = "
                         + std::to_string(record.requestId()));
        return;
    }

    switch (record.type()){
    case RecordType::BeginRequest:
        onBeginRequest(record.requestId(), record.getMessage<MsgBeginRequest>());
        break;
    case RecordType::AbortRequest:
        endRequest(record.requestId(), ProtocolStatus::RequestComplete);
        break;
    case RecordType::GetValues:
        onGetValues(record.getMessage<MsgGetValues>());
        break;
    case RecordType::Params:
        onParams(record.requestId(), record.getMessage<MsgParams>());
        break;
    case RecordType::StdIn:
        onStdIn(record.requestId(), record.getMessage<MsgStdIn>());
        break;
    default:;
    }
}

void ResponderImpl::onBeginRequest(uint16_t requestId, const MsgBeginRequest& msg)
{
    if (msg.role() != Role::Responder){
        sendMessage(requestId, MsgEndRequest{0, ProtocolStatus::UnknownRole});
        if (msg.resultConnectionState() == ResultConnectionState::Close)
            disconnect_();
        return;
    }
    if (!cfg_.multiplexingEnabled && !requestRegistry_.isEmpty() && !requestRegistry_.hasRequest(requestId)){
        sendMessage(requestId, MsgEndRequest{0, ProtocolStatus::CantMpxConn});
        if (msg.resultConnectionState() == ResultConnectionState::Close)
            disconnect_();
        return;
    }
    if (requestRegistry_.requestCount() == cfg_.maxRequestsNumber && !requestRegistry_.hasRequest(requestId)){
        sendMessage(requestId, MsgEndRequest{0, ProtocolStatus::Overloaded});
        if (msg.resultConnectionState() == ResultConnectionState::Close)
            disconnect_();
        return;
    }

    createRequest(requestId, msg.resultConnectionState() == ResultConnectionState::KeepOpen);
}

void ResponderImpl::endRequest(uint16_t requestId, ProtocolStatus protocolStatus)
{
    sendMessage(requestId, MsgEndRequest{0, protocolStatus});
    if (!requestSettingsMap_[requestId].keepConnection)
        disconnect_();

    deleteRequest(requestId);
}

void ResponderImpl::createRequest(uint16_t requestId, bool keepConnection)
{
    requestRegistry_.registerRequest(requestId);
    requestSettingsMap_[requestId].keepConnection = keepConnection;
}

void ResponderImpl::deleteRequest(uint16_t requestId)
{
    requestRegistry_.unregisterRequest(requestId);
    requestSettingsMap_.erase(requestId);
}

void ResponderImpl::onGetValues(const MsgGetValues &msg)
{
    auto result = MsgGetValuesResult{};
    for (auto request : msg.requestList()){
        switch (request){
        case ValueRequest::MaxConns:
            result.setRequestValue(request, std::to_string(cfg_.maxConnectionsNumber));
            break;
        case ValueRequest::MaxReqs:
            result.setRequestValue(request, std::to_string(cfg_.maxRequestsNumber));
            break;
        case ValueRequest::MpxsConns:
            result.setRequestValue(request, cfg_.multiplexingEnabled ? "1" : "0");
            break;
        }
    }
    sendMessage(0, std::move(result));
}

void ResponderImpl::onParams(uint16_t requestId, const MsgParams& msg)
{
    requestRegistry_.fillRequestData(requestId, msg);
}

void ResponderImpl::onStdIn(uint16_t requestId, const MsgStdIn& msg)
{
    requestRegistry_.fillRequestData(requestId, msg);
    if (msg.data().empty())
        onRequestReceived(requestId);
}

void ResponderImpl::sendRecord(const Record &record)
{
    recordStream_.seekp(0);
    recordBuffer_.resize(record.size());
    try{
        record.toStream(recordStream_);
    }
    catch (std::exception& e){
        notifyAboutError(e.what());
        return;
    }
    sendData_(recordBuffer_);
}

bool ResponderImpl::isRecordExpected(const Record& record)
{
    const auto requestRegistered = requestRegistry_.hasRequest(record.requestId());
    if (record.type() == RecordType::GetValues)
        return true;
    if (record.type() == RecordType::BeginRequest && !requestRegistered)
        return true;
    if (record.type() != RecordType::BeginRequest && requestRegistered)
        return true;
    return false;
}

void ResponderImpl::onRequestReceived(uint16_t requestId)
{
    processRequest_(requestRegistry_.makeRequest(requestId),
                    Response{[requestId, this](std::string&& data, std::string&& errorMsg) {
                        sendResponse(requestId, std::move(data), std::move(errorMsg));
                    }});
}

void ResponderImpl::sendResponse(uint16_t id, std::string&& data, std::string&& errorMsg)
{
    auto dataStream = makeStream<MsgStdOut>(id, data);
    auto errorStream = makeStream<MsgStdErr>(id, errorMsg);
    std::for_each(dataStream.begin(), dataStream.end(), [this](const Record& record){sendRecord(record);});
    std::for_each(errorStream.begin(), errorStream.end(), [this](const Record& record){sendRecord(record);});
    endRequest(id, ProtocolStatus::RequestComplete);
}

void ResponderImpl::setMaximumConnectionsNumber(int value)
{
    cfg_.maxConnectionsNumber = value;
}

void ResponderImpl::setMaximumRequestsNumber(int value)
{
    cfg_.maxRequestsNumber = value;
}

void ResponderImpl::setMultiplexingEnabled(bool state)
{
    cfg_.multiplexingEnabled = state;
}

void ResponderImpl::setErrorInfoHandler(std::function<void (const std::string &)> handler)
{
    errorInfoHandler_ = std::move(handler);
    recordReader_.setErrorInfoHandler(errorInfoHandler_);
}

int ResponderImpl::maximumConnectionsNumber() const
{
    return cfg_.maxConnectionsNumber;
}

int ResponderImpl::maximumRequestsNumber() const
{
    return cfg_.maxRequestsNumber;
}

bool ResponderImpl::isMultiplexingEnabled() const
{
    return cfg_.multiplexingEnabled;
}

void ResponderImpl::notifyAboutError(const std::string &errorMsg)
{
    if (errorInfoHandler_)
        errorInfoHandler_(errorMsg);
}

}
