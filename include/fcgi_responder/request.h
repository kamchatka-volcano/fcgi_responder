#pragma once
#include <string>
#include <unordered_map>
#include <vector>

namespace fcgi{

class MsgStdIn;
class MsgParams;

///
/// \brief Object containing request data from the web server
///
class Request{

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
    const std::string& param(const std::string& name) const;

    ///
    /// \brief paramList
    /// Returns list of environment variable names passed from the web server
    /// \return list of names
    ///
    std::vector<std::string> paramList() const;

    ///
    /// \brief params
    /// Returns params map
    /// \return
    ///
    const std::unordered_map<std::string, std::string>& params() const;

    ///
    /// \brief hasParam
    /// Returns true if request has specified parameter
    /// \param name
    /// \return
    ///
    bool hasParam(const std::string& name) const;

private:
    void addParams(const fcgi::MsgParams& msg);
    void addData(const fcgi::MsgStdIn& msg);
    friend class Responder;
    friend class RequestMaker;

private:
    std::string stdIn_;
    std::unordered_map<std::string, std::string> params_;
};

} //namespace fcgi

