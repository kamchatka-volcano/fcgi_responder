#pragma once
#include "constants.h"
#include "types.h"
#include "response.h"
#include "request.h"
#include <unordered_map>
#include <functional>
#include <memory>

namespace fcgi{

class MsgBeginRequest;
class MsgGetValues;
class MsgParams;
class MsgStdIn;
class Record;
class Message;
class RecordReader;

class Responder{
public:
    void setMaximumConnectionsNumber(int value);
    void setMaximumRequestsNumber(int value);
    void setMultiplexingEnabled(bool state);
    int maximumConnectionsNumber() const;
    int maximumRequestsNumber() const;
    bool isMultiplexingEnabled() const;
    void setErrorInfoHandler(std::function<void(const std::string&)>);

protected:
    Responder();
    virtual ~Responder();
    void receiveData(const std::string& data);
    virtual void sendData(const std::string& data) = 0;
    virtual void disconnect() = 0;
    virtual Response processRequest(const Request& request) = 0;

private:
    void onRecordReaded(const fcgi::Record& record);
    void onBeginRequest(uint16_t requestId, const fcgi::MsgBeginRequest& msg);
    void onGetValues(const fcgi::MsgGetValues& msg);
    void onParams(uint16_t requestId, const fcgi::MsgParams& msg);
    void onStdIn(uint16_t requestId, const fcgi::MsgStdIn& msg);
    void onRequestReceived(uint16_t requestId);

    void send(uint16_t requestId, const fcgi::Message& msg);
    bool isRecordExpected(const fcgi::Record& record);
    void endRequest(uint16_t requestId, fcgi::ProtocolStatus protocolStatus = fcgi::ProtocolStatus::RequestComplete);

private:
    struct Config{
        int maxConnectionsNumber = cDefaultMaxConnectionsNumber;
        int maxRequestsNumber = cDefaultMaxRequestsNumber;
        bool multiplexingEnabled = cDefaultMultiplexingEnabled;
    } cfg_;

    std::unique_ptr<RecordReader> recordReader_;
    std::unordered_map<uint16_t, Request> requestMap_;
    std::function<void(const std::string&)> errorInfoHandler_;
};

}
