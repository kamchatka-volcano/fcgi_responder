#pragma once
#include "recordreader.h"
#include "streamdatamessage.h"
#include "types.h"
#include "requestdata.h"
#include "datawriterstream.h"
#include <unordered_map>
#include <functional>
#include <memory>
#include <sstream>

namespace fcgi{
class Request;
class Response;
class MsgBeginRequest;
class MsgGetValues;
class MsgParams;
class Record;

class ResponderImpl : public std::enable_shared_from_this<ResponderImpl>{
public:
    ResponderImpl(std::function<void(const std::string&)> sendData,
                  std::function<void()> disconnect,
                  std::function<void(Request&& request, Response&& response)> processRequest);
    void receiveData(const char* data, std::size_t size);
    void setMaximumConnectionsNumber(int value);
    void setMaximumRequestsNumber(int value);
    void setMultiplexingEnabled(bool state);
    int maximumConnectionsNumber() const;
    int maximumRequestsNumber() const;
    bool isMultiplexingEnabled() const;
    void setErrorInfoHandler(std::function<void(const std::string&)> errorInfoHandler);

private:
    void onRecordRead(const Record& record);
    void onBeginRequest(uint16_t requestId, const MsgBeginRequest& msg);
    void onGetValues(const MsgGetValues& msg);
    void onParams(uint16_t requestId, const MsgParams& msg);
    void onStdIn(uint16_t requestId, const StreamDataMessage<RecordType::StdIn>& msg);
    void onRequestReceived(uint16_t requestId);
    void sendRecord(const Record& record);
    void sendResponse(uint16_t id, std::string&& data, std::string&& errorMsg);

    bool isRecordExpected(const Record& record);
    void endRequest(uint16_t requestId);

    void notifyAboutError(const std::string& errorMsg);
    void createRequest(uint16_t requestId, bool keepConnection);
    void deleteRequest(uint16_t requestId);

private:
    struct Config{
        int maxConnectionsNumber = 1;
        int maxRequestsNumber = 10;
        bool multiplexingEnabled = true;
    } cfg_;

    RecordReader recordReader_;
    std::unordered_map<uint16_t, RequestData> requestRegistry_;
    std::function<void(const std::string&)> errorInfoHandler_;
    DataWriterStream recordStream_;
    std::function<void(const std::string&)> sendData_;
    std::function<void()> disconnect_;
    std::function<void(Request&& request, Response&& response)> processRequest_;

private:
    template<typename TMsg>
    void sendMessage(uint16_t requestId, TMsg&& msg);
};

}
