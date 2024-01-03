#pragma once
#include "datawriterstream.h"
#include "recordreader.h"
#include "streamdatamessage.h"
#include <fcgi_responder/requester.h>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <sstream>
#include <string>

namespace fcgi {
class RecordReader;
class Record;
class Request;
class Response;
class MsgGetValuesResult;
class MsgUnknownType;
class MsgEndRequest;

class RequesterImpl {
    enum class ConnectionState {
        NotConnected,
        ConnectionInProgress,
        Connected
    };

    enum class ResponseStatus {
        Successful,
        Failed,
        Cancelled
    };

public:
    RequesterImpl(std::function<void(const std::string&)> sendData, std::function<void()> disconnect);
    void receiveData(const char* data, std::size_t size);
    std::optional<RequestHandle> sendRequest(
            std::map<std::string, std::string> params,
            std::string data,
            const std::function<void(std::optional<ResponseData>)>& responseHandler,
            bool keepConnection = false);
    void setErrorInfoHandler(const std::function<void(const std::string&)>& handler);

    int availableRequestsNumber() const;
    int maximumConnectionsNumber() const;
    int maximumRequestsNumber() const;
    bool isMultiplexingEnabled() const;

private:
    void initConnection(
            std::map<std::string, std::string> params,
            std::string data,
            std::function<void(std::optional<ResponseData>)> responseHandler,
            bool keepConnection);
    std::optional<RequestHandle> doSendRequest(
            const std::map<std::string, std::string>& params,
            const std::string& data,
            std::function<void(std::optional<ResponseData>)> responseHandler,
            bool keepConnection);
    void doEndRequest(std::uint16_t requestId, ResponseStatus responseStatus);
    void onRecordRead(const Record& record);
    template<typename TMsg>
    void sendMessage(std::uint16_t requestId, TMsg&& msg);
    void notifyAboutError(const std::string& errorMsg);
    void sendRecord(const Record& record);
    bool isRecordExpected(const Record& record);
    void onGetValuesResult(const MsgGetValuesResult& msg);
    void onUnknownType(std::uint16_t requestId, const MsgUnknownType& msg);
    void onEndRequest(std::uint16_t requestId, const MsgEndRequest& msg);
    void onStdOut(std::uint16_t requestId, const MsgStdOut& msg);
    void onStdErr(std::uint16_t requestId, const MsgStdErr& msg);

private:
    struct Config {
        int maxConnectionsNumber = 1;
        int maxRequestsNumber = 10;
        bool multiplexingEnabled = true;
    } cfg_;

    struct ResponseContext {
        std::function<void(std::optional<ResponseData>)> responseHandler;
        ResponseData responseData;
        bool keepConnection = false;
        std::shared_ptr<std::function<void()>> cancelRequestHandler;
    };

    RecordReader recordReader_;
    DataWriterStream recordStream_;
    std::function<void(const std::string&)> errorInfoHandler_;
    std::function<void()> onConnectionFail_;
    std::function<void()> onConnectionSuccess_;
    std::shared_ptr<std::function<void()>> connectionOpeningRequestCancelHandler_;
    ConnectionState connectionState_ = ConnectionState::NotConnected;
    std::set<std::uint16_t> requestIdPool_;
    std::map<std::uint16_t, ResponseContext> responseMap_;
    std::function<void(const std::string&)> sendData_;
    std::function<void()> disconnect_;
};

} //namespace fcgi
