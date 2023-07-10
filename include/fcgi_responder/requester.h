#pragma once
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>

namespace fcgi {
class RequesterImpl;

class RequestHandle {
public:
    RequestHandle(const std::shared_ptr<std::function<void()>>& cancelRequestHandler);
    void cancelRequest();

private:
    std::weak_ptr<std::function<void()>> cancelRequestHandler_;
};

struct ResponseData {
    std::string data;
    std::string errorMsg;
};

///
/// \brief Abstract class which implements the client logic for making requests to FastCGI applications
///
class Requester {

public:
    ///
    /// \brief sendRequest
    /// Send request to the FastCGI application
    /// \param params request parameters
    /// \param data request data
    /// \param responseHandler response handler
    /// \param keepConnection true if FastCGI application should keep the connection alive after response is sent
    /// \return RequestHandle - object which can be used to cancel request
    ///
    std::optional<RequestHandle> sendRequest(
            std::map<std::string, std::string> params,
            std::string data,
            const std::function<void(std::optional<ResponseData>)>& responseHandler,
            bool keepConnection = false);
    ///
    /// \brief setErrorInfoHandler
    /// Protocol and stream errors are handled internally and silently,
    /// this method can be used to get the text information about these errors,
    /// by registering a handler function.
    /// \param errorInfoHandler
    ///
    void setErrorInfoHandler(const std::function<void(const std::string&)>& handler);

    ///
    /// \brief availableRequestsNumber
    /// \return number of available requests
    int availableRequestsNumber() const;

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

    Requester(const Requester&) = delete;
    Requester& operator=(const Requester&) = delete;

protected:
    Requester();
    virtual ~Requester();
    Requester(Requester&&) = default;
    Requester& operator=(Requester&&) = default;

    void receiveData(const char* data, std::size_t size);
    virtual void sendData(const std::string& data) = 0;
    virtual void disconnect() = 0;

private:
    RequesterImpl& impl();
    const RequesterImpl& impl() const;

private:
    std::unique_ptr<RequesterImpl> impl_;
};

} //namespace fcgi
