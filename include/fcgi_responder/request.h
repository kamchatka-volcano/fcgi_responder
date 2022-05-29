#pragma once
#include <string>
#include <vector>

namespace fcgi{

///
/// \brief Object containing request data from the web server
///
class Request{

public:
    ///
    /// \brief stdIn_
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
    const std::string& param(const std::string& name) const;

    ///
    /// \brief params
    /// Returns list of environment variable name-value pairs passed from the web server
    /// \return list of name-value pairs
    ///
    const std::vector<std::pair<std::string, std::string>>& params() const;

    ///
    /// \brief hasParam
    /// Returns true if request has specified parameter
    /// \param name
    /// \return
    ///
    bool hasParam(const std::string& name) const;

    ///
    /// \brief Constructor
    /// \param params fcgi parameters
    /// \param stdIn request data
    Request(std::vector<std::pair<std::string, std::string>> params, std::string stdIn);

private:
    std::vector<std::pair<std::string, std::string>> params_;
    std::string stdIn_;

};

} //namespace fcgi

