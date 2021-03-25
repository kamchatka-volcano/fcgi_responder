#pragma once
#include "response.h"
#include "request.h"
#include <unordered_map>
#include <functional>
#include <memory>
#include <sstream>

namespace fcgi{

class MsgBeginRequest;
class MsgGetValues;
class MsgParams;
class MsgStdIn;
class Record;
class RecordReader;
enum class ProtocolStatus : uint8_t;

///
/// \brief Abstract class which implements message flow of the FastCGI protocol's
/// Responder role between application and web server.
///
class Responder{

public:
    ///
    /// \brief setsMaximumConnectionsNumber
    /// Sets a maximum connection number.
    /// \param value
    ///
    void setMaximumConnectionsNumber(int value);

    ///
    /// \brief setMaximumRequestsNumber
    /// Sets a maximum requests number.
    /// Incoming requests are dropped when their number exceeds this limit.
    /// \param value
    ///
    void setMaximumRequestsNumber(int value);

    ///
    /// \brief setMultiplexingEnabled
    /// Enables or disables multiplexing of requests.
    /// When it's enabled Responder can process <maximumRequestNumber> of requests at a time, otherwise
    /// it can process only a single request per connection.
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
    /// this method can be used to get the text information about these errors,
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
    void receiveData(const char* data, std::size_t size);

    ///
    /// \brief sendData
    /// Implement this method to send response data to the web server
    /// \param data
    ///
    virtual void sendData(const std::string& data) = 0;

    ///
    /// \brief disconnect
    /// Implement this method to close the current connection with the web server
    ///
    virtual void disconnect() = 0;

    ///
    /// \brief processRequest
    /// Implement this method to form response data for the web server
    /// and send it using the fcgi::Response object
    /// \param request
    /// \param response
    ///
    virtual void processRequest(Request&& request, Response&& response) = 0;


private:
    void onRecordReaded(const Record& record);
    void onBeginRequest(uint16_t requestId, const MsgBeginRequest& msg);
    void onGetValues(const MsgGetValues& msg);
    void onParams(uint16_t requestId, const MsgParams& msg);
    void onStdIn(uint16_t requestId, const MsgStdIn& msg);
    void onRequestReceived(uint16_t requestId);
    void sendRecord(const Record& record);
    void sendResponse(uint16_t id, std::string&& data, std::string&& errorMsg);

    bool isRecordExpected(const Record& record);
    void endRequest(uint16_t requestId, ProtocolStatus protocolStatus);

    void notifyAboutError(const std::string& errorMsg);
    void createRequest(uint16_t requestId, bool keepConnection);
    void deleteRequest(uint16_t requestId);

private:
    struct Config{
        int maxConnectionsNumber = 1;
        int maxRequestsNumber = 10;
        bool multiplexingEnabled = true;
    } cfg_;

    struct RequestSettings
    {
        bool keepConnection = true;
    };

    std::unique_ptr<RecordReader> recordReader_;
    std::unordered_map<uint16_t, Request> requestMap_;
    std::unordered_map<uint16_t, RequestSettings> requestSettingsMap_;
    std::function<void(const std::string&)> errorInfoHandler_;
    std::ostringstream recordStream_;
    std::string recordBuffer_;

private:
    template<typename TMsg>
    friend void sendMessage(Responder& responder, uint16_t requestId, TMsg&& msg);
};

}
