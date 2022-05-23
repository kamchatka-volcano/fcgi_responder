#include <fcgi_responder/requester.h>
#include "recordreader.h"
#include "record.h"
#include <cstdint>

namespace fcgi{

namespace {
    std::set<uint16_t> generateRequestIds(int maxRequestsNumber)
    {
        auto result = std::set<uint16_t>{};
        for (auto i = 1; i <= maxRequestsNumber; ++i)
            result.insert(static_cast<uint16_t>(i));
        return result;
    }
}

RequestHandle::RequestHandle(const std::shared_ptr<std::function<void()>>& cancelRequestHandler)
    : cancelRequestHandler_(cancelRequestHandler)
{
}

void RequestHandle::cancelRequest()
{
    if (auto cancelRequestHandler = cancelRequestHandler_.lock())
        (*cancelRequestHandler)();
}

Requester::Requester()
    : recordReader_(std::make_unique<RecordReader>([this](const Record& record)
      {
          onRecordRead(record);
      }))
{
    recordBuffer_.resize(cMaxRecordSize);
    recordStream_.rdbuf()->pubsetbuf(&recordBuffer_[0], cMaxRecordSize);
}

Requester::~Requester() = default;

std::optional<RequestHandle> Requester::sendRequest(
        const std::map<std::string, std::string>& params,
        const std::string& data,
        const std::function<void(const std::optional<ResponseData>&)>& responseHandler,
        bool keepConnection)
{
    if (connectionState_ == ConnectionState::NotConnected) {
        initConnection(params, data, responseHandler, keepConnection);
        connectionOpeningRequestCancelHandler_ = std::make_shared<std::function<void()>>(
           [=]{
               errorInfoHandler_("Connection initialization cancelled");
               connectionState_ = ConnectionState::NotConnected;
               responseHandler(std::nullopt);
           });
        return connectionOpeningRequestCancelHandler_;
    }
    else if (connectionState_ == ConnectionState::Connected)
        return doSendRequest(params, data, responseHandler, keepConnection);

    return std::nullopt;
}

int Requester::availableRequestsNumber() const
{
    switch (connectionState_)
    {
    case ConnectionState::NotConnected:
        return 1;
    case ConnectionState::ConnectionInProgress:
        return 0;
    case ConnectionState::Connected:
        return static_cast<int>(requestIdPool_.size());
    }
}

void Requester::initConnection(
        const std::map<std::string, std::string>& params,
        const std::string& data,
        const std::function<void(const std::optional<ResponseData>&)>& responseHandler,
        bool keepConnection)
{
    connectionState_ = ConnectionState::ConnectionInProgress;
    onConnectionFail_ = [=]() {
        connectionState_ = ConnectionState::NotConnected;
        responseHandler(std::nullopt);
    };
    onConnectionSuccess_ = [=]() {
        connectionState_ = ConnectionState::Connected;
        requestIdPool_ = cfg_.multiplexingEnabled ? generateRequestIds(cfg_.maxRequestsNumber) : std::set<uint16_t>{1};
        doSendRequest(params, data, responseHandler, keepConnection);
        *connectionOpeningRequestCancelHandler_ = *responseMap_.begin()->second.cancelRequestHandler;
    };
    auto getValuesMsg = MsgGetValues{};
    getValuesMsg.requestValue(ValueRequest::MaxReqs);
    getValuesMsg.requestValue(ValueRequest::MpxsConns);
    sendMessage(0, getValuesMsg);
}


std::optional<RequestHandle> Requester::doSendRequest(
        const std::map<std::string, std::string>& params,
        const std::string& data,
        const std::function<void(const std::optional<ResponseData>&)>& responseHandler,
        bool keepConnection)
{
    if (requestIdPool_.empty()){
        errorInfoHandler_("Maximum requests number reached");
        responseHandler(std::nullopt);
        return std::nullopt;
    }

    auto requestId = *requestIdPool_.begin();
    requestIdPool_.erase(requestId);
    responseMap_.emplace(requestId,
                         ResponseContext{responseHandler, ResponseData{}, keepConnection,
                                         std::make_shared<std::function<void()>>([=] {
                                             doEndRequest(requestId, ResponseStatus::Cancelled);
                                         })
                         });

    sendMessage(requestId, MsgBeginRequest{Role::Responder,
                                           keepConnection ? ResultConnectionState::KeepOpen :
                                                            ResultConnectionState::Close});
    auto paramsMsg = MsgParams{};
    for (const auto& [paramName, paramValue] : params)
        paramsMsg.setParam(paramName, paramValue);
    sendMessage(requestId, paramsMsg);
    if (!params.empty())
        sendMessage(requestId, MsgParams{});
    sendMessage(requestId, MsgStdIn{data});
    if (!data.empty())
        sendMessage(requestId, MsgStdIn{""});

    return responseMap_.at(requestId).cancelRequestHandler;
}

void Requester::doEndRequest(uint16_t requestId, ResponseStatus responseStatus)
{
    const auto& responseContext = responseMap_.at(requestId);
    if (responseStatus == ResponseStatus::Successful)
        responseContext.responseHandler(responseContext.responseData);
    else
        responseContext.responseHandler(std::nullopt);

    if (responseStatus != ResponseStatus::Cancelled && !responseContext.keepConnection)
        disconnect();

    responseMap_.erase(requestId);
    requestIdPool_.insert(requestId);
}

template <typename TMsg>
void Requester::sendMessage(uint16_t requestId, TMsg&& msg)
{
    auto record = Record{std::forward<TMsg>(msg), requestId};
    sendRecord(record);
}
template void Requester::sendMessage<MsgGetValues>(uint16_t requestId, MsgGetValues&& msg);
template void Requester::sendMessage<MsgBeginRequest>(uint16_t requestId, MsgBeginRequest&& msg);
template void Requester::sendMessage<MsgAbortRequest>(uint16_t requestId, MsgAbortRequest&& msg);
template void Requester::sendMessage<MsgParams>(uint16_t requestId, MsgParams&& msg);
template void Requester::sendMessage<MsgStdIn>(uint16_t requestId, MsgStdIn&& msg);

void Requester::sendRecord(const Record &record)
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
    sendData(recordBuffer_);
}

void Requester::receiveData(const char* data, std::size_t size)
{
    try{
        recordReader_->read(data, size);
    }
    catch(const InvalidRecordType& e){
        notifyAboutError(e.what());
        recordReader_->removeBrokenRecord(e.recordSize(), data, size);

    }
    catch(const RecordReadError& e){
        notifyAboutError(e.what());
        recordReader_->removeBrokenRecord(e.recordSize(), data, size);
    }
    catch(const std::exception& e){
        notifyAboutError(e.what());
        recordReader_->clear();
    }
}

bool Requester::isRecordExpected(const Record& record)
{
    if (record.type() == RecordType::GetValuesResult)
        return true;
    if (record.type() == RecordType::UnknownType ||
        record.type() == RecordType::StdOut ||
        record.type() == RecordType::StdErr ||
        record.type() == RecordType::EndRequest)
        return responseMap_.count(record.requestId());

    return false;
}

void Requester::onRecordRead(const Record& record)
{
    if (!isRecordExpected(record)) {
        notifyAboutError("Received unexpected record, RecordType = "
                         + std::to_string(static_cast<int>(record.type()))
                         + ", requestId = "
                         + std::to_string(record.requestId()));
        return;
    }

    switch (record.type()){
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

void Requester::onGetValuesResult(const MsgGetValuesResult &msg)
{
    for (auto request : msg.requestList()){
        switch (request){
        case ValueRequest::MaxReqs:
            try {
                cfg_.maxRequestsNumber = std::stoi(msg.requestValue(request));
            }
            catch (std::exception& e){
                notifyAboutError("Invalid value for MaxReqs: " + msg.requestValue(request));
                onConnectionFail_();
                return;
            }
            break;
        case ValueRequest::MpxsConns:
            try {
                cfg_.multiplexingEnabled = std::stoi(msg.requestValue(request)) ? true : false;
            }
            catch (std::exception& e){
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

void Requester::onUnknownType(uint16_t requestId, const MsgUnknownType& msg)
{
    notifyAboutError("Received unknown record type: " + std::to_string(msg.unknownTypeValue()));
    sendMessage(requestId, MsgAbortRequest{});
}

void Requester::onEndRequest(uint16_t requestId, const MsgEndRequest& msg)
{
    doEndRequest(requestId, msg.protocolStatus() == ProtocolStatus::RequestComplete ? ResponseStatus::Successful
                                                                                    : ResponseStatus::Failed);
}

void Requester::onStdOut(uint16_t requestId, const MsgStdOut& msg)
{
    responseMap_.at(requestId).responseData.data += msg.data();
}

void Requester::onStdErr(uint16_t requestId, const MsgStdErr& msg)
{
    responseMap_.at(requestId).responseData.errorMsg += msg.data();
}

void Requester::setErrorInfoHandler(const std::function<void (const std::string &)>& handler)
{
    errorInfoHandler_ = handler;
}

void Requester::notifyAboutError(const std::string &errorMsg)
{
    if (errorInfoHandler_)
        errorInfoHandler_(errorMsg);
}

int Requester::maximumConnectionsNumber() const
{
    return cfg_.maxConnectionsNumber;
}

int Requester::maximumRequestsNumber() const
{
    return cfg_.maxRequestsNumber;
}

bool Requester::isMultiplexingEnabled() const
{
    return cfg_.multiplexingEnabled;
}

}