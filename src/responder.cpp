#include "responder.h"
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
#include "requesteditor.h"
#include "recordreader.h"
#include "constants.h"
#include <algorithm>

namespace fcgi{

Responder::Responder()
    : recordReader_(std::make_unique<RecordReader>([this](const Record& record)
      {
          onRecordReaded(record);
      }))
{
    recordBuffer_.resize(cMaxRecordSize);
    recordStream_.rdbuf()->pubsetbuf(&recordBuffer_[0], cMaxRecordSize);
}

Responder::~Responder()
{}

void Responder::receiveData(const char* data, std::size_t size)
{
    try{
        recordReader_->addData(data, size);
    }
    catch(const InvalidRecordType& e){        
        notifyAboutError(e.what());
        send(0, std::make_unique<MsgUnknownType>(e.recordType()));
        recordReader_->removeBrokenRecord(e.recordSize());

    }
    catch(const RecordReadError& e){
        notifyAboutError(e.what());
        recordReader_->removeBrokenRecord(e.recordSize());
    }
    catch(const std::exception& e){
        notifyAboutError(e.what());
        recordReader_->clear();
    }
}

void Responder::onRecordReaded(const Record& record)
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

void Responder::onBeginRequest(uint16_t requestId, const MsgBeginRequest& msg)
{
    if (msg.role() != Role::Responder){
        send(requestId, std::make_unique<MsgEndRequest>(0, ProtocolStatus::UnknownRole));
        if (msg.resultConnectionState() == ResultConnectionState::Close)
            disconnect();
        return;
    }
    if (!cfg_.multiplexingEnabled && requestMap_.size() > 0 && !requestMap_.count(requestId)){
        send(requestId, std::make_unique<MsgEndRequest>(0, ProtocolStatus::CantMpxConn));
        if (msg.resultConnectionState() == ResultConnectionState::Close)
            disconnect();
        return;
    }
    if (requestMap_.size() == static_cast<std::size_t>(cfg_.maxRequestsNumber) && !requestMap_.count(requestId)){
        send(requestId, std::make_unique<MsgEndRequest>(0, ProtocolStatus::Overloaded));
        if (msg.resultConnectionState() == ResultConnectionState::Close)
            disconnect();
        return;
    }
    RequestEditor(requestMap_[requestId]).setKeepConnection(
                msg.resultConnectionState() == ResultConnectionState::KeepOpen);
}

void Responder::endRequest(uint16_t requestId, ProtocolStatus protocolStatus)
{
    send(requestId, std::make_unique<MsgEndRequest>(0, protocolStatus));
    if (!requestMap_[requestId].keepConnection())
        disconnect();

    requestMap_.erase(requestId);
}

void Responder::onGetValues(const MsgGetValues &msg)
{
    auto result = std::make_unique<MsgGetValuesResult>();
    for (auto request : msg.requestList()){
        switch (request){
        case ValueRequest::MaxConns:
            result->setRequestValue(request, std::to_string(cfg_.maxConnectionsNumber));
            break;
        case ValueRequest::MaxReqs:
            result->setRequestValue(request, std::to_string(cfg_.maxRequestsNumber));
            break;
        case ValueRequest::MpxsConns:
            result->setRequestValue(request, cfg_.multiplexingEnabled ? "1" : "0");
            break;
        }
    }
    send(0, std::move(result));
}

void Responder::onParams(uint16_t requestId, const MsgParams& msg)
{
    RequestEditor{requestMap_[requestId]}.addParamsMsg(msg);
}

void Responder::onStdIn(uint16_t requestId, const MsgStdIn& msg)
{
    RequestEditor{requestMap_[requestId]}.addStdInMsg(msg);
    if (msg.data().empty())
        onRequestReceived(requestId);
}

void Responder::send(uint16_t requestId, std::unique_ptr<Message> msg)
{
    auto record = Record{std::move(msg), requestId};
    sendRecord(record);
}

void Responder::sendRecord(const Record &record)
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

bool Responder::isRecordExpected(const Record& record)
{
    auto requestRegistered = requestMap_.find(record.requestId()) != requestMap_.end();
    if (record.type() == RecordType::GetValues)
        return true;
    if (record.type() == RecordType::BeginRequest && !requestRegistered)
        return true;
    if (record.type() != RecordType::BeginRequest && requestRegistered)
        return true;
    return false;
}

void Responder::onRequestReceived(uint16_t requestId)
{
    auto response = processRequest(requestMap_[requestId]);
    auto streamMaker = StreamMaker{};
    auto dataStream = streamMaker.makeStream(RecordType::StdOut, requestId, response.moveOutData());
    auto errorStream = streamMaker.makeStream(RecordType::StdErr, requestId, response.moveOutErrorMsg());
    std::for_each(dataStream.begin(), dataStream.end(), [this](const Record& record){sendRecord(record);});
    std::for_each(errorStream.begin(), errorStream.end(), [this](const Record& record){sendRecord(record);});
    endRequest(requestId, ProtocolStatus::RequestComplete);
}

void Responder::setMaximumConnectionsNumber(int value)
{
    cfg_.maxConnectionsNumber = value;
}

void Responder::setMaximumRequestsNumber(int value)
{
    cfg_.maxRequestsNumber = value;
}

void Responder::setMultiplexingEnabled(bool state)
{
    cfg_.multiplexingEnabled = state;
}

void Responder::setErrorInfoHandler(std::function<void (const std::string &)> handler)
{
    errorInfoHandler_ = handler;
}

int Responder::maximumConnectionsNumber() const
{
    return cfg_.maxConnectionsNumber;
}

int Responder::maximumRequestsNumber() const
{
    return cfg_.maxRequestsNumber;
}

bool Responder::isMultiplexingEnabled() const
{
    return cfg_.multiplexingEnabled;
}

void Responder::notifyAboutError(const std::string &errorMsg)
{
    if (errorInfoHandler_)
        errorInfoHandler_(errorMsg);
}

}
