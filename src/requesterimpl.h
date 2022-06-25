#pragma once
#include <fcgi_responder/requester.h>
#include "recordreader.h"
#include "streamdatamessage.h"
#include <string>
#include <sstream>
#include <memory>
#include <functional>
#include <set>
#include <map>
#include <optional>
#include <memory>

namespace fcgi{
class RecordReader;
class Record;
class Request;
class Response;
class MsgGetValuesResult;
class MsgUnknownType;
class MsgEndRequest;

class RequesterImpl{
    enum class ConnectionState{
        NotConnected,
        ConnectionInProgress,
        Connected
    };

    enum class ResponseStatus{
        Successful,
        Failed,
        Cancelled
    };

public:
    RequesterImpl(std::function<void(const std::string&)> sendData,
                  std::function<void()> disconnect);
    void receiveData(const char* data, std::size_t size);
    std::optional<RequestHandle> sendRequest(
            const std::map<std::string, std::string>& params, const std::string& data,
            const std::function<void(const std::optional<ResponseData>&)>& responseHandler,
            bool keepConnection = false);
    void setErrorInfoHandler(const std::function<void (const std::string &)>& handler);

    int availableRequestsNumber() const;
    int maximumConnectionsNumber() const;
    int maximumRequestsNumber() const;
    bool isMultiplexingEnabled() const;


private:
    void initConnection(
            const std::map<std::string, std::string>& params,
            const std::string& data,
            const std::function<void(const std::optional<ResponseData>&)>& responseHandler,
            bool keepConnection);
    std::optional<RequestHandle> doSendRequest(
            const std::map<std::string, std::string>& params,
            const std::string& data,
            const std::function<void(const std::optional<ResponseData>&)>& responseHandler,
            bool keepConnection);
    void doEndRequest(uint16_t requestId, ResponseStatus responseStatus);
    void onRecordRead(const Record& record);
    template <typename TMsg>
    void sendMessage(uint16_t requestId, TMsg&& msg);
    void notifyAboutError(const std::string &errorMsg);
    void sendRecord(const Record &record);
    bool isRecordExpected(const Record& record);
    void onGetValuesResult(const MsgGetValuesResult& msg);
    void onUnknownType(uint16_t requestId, const MsgUnknownType& msg);
    void onEndRequest(uint16_t requestId, const MsgEndRequest& msg);
    void onStdOut(uint16_t requestId, const MsgStdOut& msg);
    void onStdErr(uint16_t requestId, const MsgStdErr& msg);

private:
    struct Config{
        int maxConnectionsNumber = 1;
        int maxRequestsNumber = 10;
        bool multiplexingEnabled = true;
    } cfg_;

    struct ResponseContext{
        std::function<void(const std::optional<ResponseData>&)> responseHandler;
        ResponseData responseData;
        bool keepConnection = false;
        std::shared_ptr<std::function<void()>> cancelRequestHandler;
    };

    RecordReader recordReader_;
    std::ostringstream recordStream_;
    std::string recordBuffer_;
    std::function<void(const std::string&)> errorInfoHandler_;
    std::function<void()> onConnectionFail_;
    std::function<void()> onConnectionSuccess_;
    std::shared_ptr<std::function<void()>> connectionOpeningRequestCancelHandler_;
    ConnectionState connectionState_ = ConnectionState::NotConnected;
    std::set<uint16_t> requestIdPool_;
    std::map<uint16_t, ResponseContext> responseMap_;
    std::function<void(const std::string&)> sendData_;
    std::function<void()> disconnect_;
};

}
