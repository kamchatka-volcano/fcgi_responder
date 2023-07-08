#pragma once
#include "request.h"
#include "response.h"
#include <memory>

namespace fcgi{

class ResponderImpl;

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

    Responder(const Responder&) = delete;
    Responder& operator=(const Responder&) = delete;

protected:
    Responder();
    virtual ~Responder();
    Responder(Responder&& other) = default;
    Responder& operator=(Responder&& other) = default;

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
    ResponderImpl& impl();
    const ResponderImpl& impl() const;

private:
    std::shared_ptr<ResponderImpl> impl_;
};

}
