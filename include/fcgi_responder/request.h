#pragma once
#include <string>
#include <unordered_map>
#include <vector>

namespace fcgi{

///
/// \brief Object containing request data from the web server
///
class Request{
    friend class RequestEditor;

public:
    ///
    /// \brief stdIn
    /// HTTP request data
    /// \return
    ///
    const std::string& stdIn() const;
    ///
    /// \brief param
    /// Returns environment variable passed from the web server
    /// \param name variable name
    /// \return variable value
    ///
    std::string param(const std::string& name) const;

    ///
    /// \brief paramList
    /// Returns list of environment variable names passed from the web server
    /// \return list of names
    ///
    std::vector<std::string> paramList() const;

    ///
    /// \brief keepConnection
    /// State of connection after the end of request's processing.
    /// If true, connection stays open, otherwise it's closed after sending the response.
    /// \return state
    ///
    bool keepConnection() const;

private:
    std::string stdIn_;
    std::unordered_map<std::string, std::string> params_;
    bool keepConnection_ = true;
};

} //namespace fcgi

