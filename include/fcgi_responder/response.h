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
    /// \brief Response
    /// Constructor
    /// \param data - HTTP response
    /// \param errorMsg - error information
    ///
    Response(std::string&& data,
             std::string&& errorMsg);

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
    /// Returns HTTP response data
    /// \return
    ///
    const std::string& data() const;

    ///
    /// \brief errorMsg
    /// Returns error information
    /// \return
    ///
    const std::string& errorMsg() const;

    ///
    /// \brief moveOutData
    /// Returns HTTP response data (moved out)
    /// \return
    ///
    std::string&& moveOutData();

    ///
    /// \brief moveOutErrorMsg
    /// Returns error information (moved out)
    /// \return
    ///
    std::string&& moveOutErrorMsg();

private:
    std::string data_;
    std::string errorMsg_;
};

}
