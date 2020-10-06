#pragma once
#include <string>

namespace fcgi{

///
/// \brief Object containing response data from the application
///
class Response{
public:
    ///
    /// \brief Response
    /// Conctructor
    ///
    Response();

    ///
    /// \brief Response
    /// Constructor
    /// \param data - HTTP response
    /// \param errorMsg - error information
    ///
    Response(const std::string& data,
             const std::string& errorMsg);

    ///
    /// \brief setData
    /// Sets HTTP response data
    /// \param data
    ///
    void setData(const std::string& data);

    ///
    /// \brief setErrorMsg
    /// Sets error information
    /// \param errorMsg
    ///
    void setErrorMsg(const std::string& errorMsg);

    ///
    /// \brief data
    /// \return HTTP response data
    ///
    const std::string& data() const;

    ///
    /// \brief errorMsg
    /// \return Error information
    ///
    const std::string& errorMsg() const;

private:
    std::string data_;
    std::string errorMsg_;
};

}
