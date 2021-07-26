#pragma once
#include <string>
#include <functional>

namespace fcgi{

///
/// \brief Move-only object used to send response data from the application
///
class Response{    
public:
    ///
    /// \brief Destructor
    /// calls the send() method
    ///
    ~Response();
    Response(Response&&) = default;
    Response& operator=(Response&&) = default;

    ///
    /// \brief setData
    /// Sets HTTP response data
    /// \param data
    ///
    void setData(std::string data);

    ///
    /// \brief setErrorMsg
    /// Sets error information
    /// \param errorMsg
    ///
    void setErrorMsg(std::string errorMsg);

    ///
    /// \brief send
    /// Sends the response data during the first call only,
    /// the next calls of this method do nothing.
    /// If a call to this method is omitted during the response object's lifetime,
    /// a call from the object's destructor will send the response data.
    ///
    void send();

    ///
    /// \brief isValid
    /// \return check whether the response is valid and can be sent
    ///
    bool isValid() const;

    ///
    /// \brief operator bool()
    /// \return check whether the response is valid and can be sent
    ///
    operator bool() const;

private:
    using ResponseSender = std::function<void(std::string&& data, std::string&& errorData)>;
    explicit Response(ResponseSender sender);
    friend class Responder;
    friend class ResponseMaker;

private:
    std::string data_;
    std::string errorMsg_;
    ResponseSender sender_;
};

}
