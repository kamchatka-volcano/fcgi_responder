#pragma once
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
enum class ProtocolStatus : uint8_t;

///
/// \brief Abstract class which implements message flow of FCGI protocol's
/// Responder role between application and web server.
///
class Responder{
public:
    ///
    /// \brief setsMaximumConnectionsNumber
    /// Sets maximum connection number.
    /// \param value
    ///
    void setMaximumConnectionsNumber(int value);

    ///
    /// \brief setMaximumRequestsNumber
    /// Sets maximum requests number.
    /// Incoming requests are dropped when their number exceeds this limit.
    /// \param value
    ///
    void setMaximumRequestsNumber(int value);

    ///
    /// \brief setMultiplexingEnabled
    /// Enables or disables multiplexing of requests.
    /// When it's enabled Responder can process <maximumRequestNumber> of requests at a time, otherwise
    /// it can process only one request per connection.
    /// \param state
    ///
    void setMultiplexingEnabled(bool state);

    ///
    /// \brief maximumConnectionsNumber
    /// \return Maximum connections number
    ///
    int maximumConnectionsNumber() const;

    ///
    /// \brief maximumRequestsNumber
    /// \return Maximum requests number
    ///
    int maximumRequestsNumber() const;

    ///
    /// \brief isMultiplexingEnabled
    /// \return Multiplexing state
    ///
    bool isMultiplexingEnabled() const;

    ///
    /// \brief setErrorInfoHandler
    /// Protocol and stream errors are handled internally and silently,
    /// this method can be used to get text information about these errors,
    /// by registering a handler function.
    /// \param errorInfoHandler
    ///
    void setErrorInfoHandler(std::function<void(const std::string&)> errorInfoHandler);

protected:
    ///
    /// \brief Responder
    /// Constructor
    ///
    Responder();

    ///
    /// \brief ~Responder
    /// Destructor
    ///
    virtual ~Responder();

    ///
    /// \brief receiveData
    /// All data received from the web server
    /// must be passed to Responder with this method
    /// \param data
    ///
    void receiveData(const std::string& data);

    ///
    /// \brief sendData
    /// Override this method to send response data to the web server
    /// \param data
    ///
    virtual void sendData(const std::string& data) = 0;

    ///
    /// \brief disconnect
    /// Override this method to close connection with the web server
    ///
    virtual void disconnect() = 0;

    ///
    /// \brief processRequest
    /// Override this method to form response data for the web server
    /// and return it inside fcgi::Repsonse object;
    /// \param request
    /// \return response data
    ///
    virtual Response processRequest(const Request& request) = 0;

private:
    void onRecordReaded(const Record& record);
    void onBeginRequest(uint16_t requestId, const MsgBeginRequest& msg);
    void onGetValues(const MsgGetValues& msg);
    void onParams(uint16_t requestId, const MsgParams& msg);
    void onStdIn(uint16_t requestId, const MsgStdIn& msg);
    void onRequestReceived(uint16_t requestId);

    void send(uint16_t requestId, std::unique_ptr<Message> msg);
    bool isRecordExpected(const Record& record);
    void endRequest(uint16_t requestId, ProtocolStatus protocolStatus);

private:
    struct Config{
        int maxConnectionsNumber = 1;
        int maxRequestsNumber = 10;
        bool multiplexingEnabled = true;
    } cfg_;

    std::unique_ptr<RecordReader> recordReader_;
    std::unordered_map<uint16_t, Request> requestMap_;
    std::function<void(const std::string&)> errorInfoHandler_;
};

}
