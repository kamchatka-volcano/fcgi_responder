#pragma once
#include <string>
#include <memory>
#include <functional>
#include <map>
#include <optional>
#include <memory>

namespace fcgi{
class RequesterImpl;

class RequestHandle{
public:
    RequestHandle(const std::shared_ptr<std::function<void()>>& cancelRequestHandler);
    void cancelRequest();

private:
    std::weak_ptr<std::function<void()>> cancelRequestHandler_;
};

struct ResponseData
{
    std::string data;
    std::string errorMsg;
};

class Requester{

public:
    std::optional<RequestHandle> sendRequest(
            const std::map<std::string, std::string>& params, const std::string& data,
            const std::function<void(const std::optional<ResponseData>&)>& responseHandler,
            bool keepConnection = false);
    void setErrorInfoHandler(const std::function<void (const std::string &)>& handler);

    int availableRequestsNumber() const;
    int maximumConnectionsNumber() const;
    int maximumRequestsNumber() const;
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

}
