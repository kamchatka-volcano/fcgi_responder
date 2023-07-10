#pragma once
#include "streamdatamessage.h"
#include <optional>
#include <string>
#include <vector>

namespace fcgi {
class MsgParams;
class Request;

class RequestData {
public:
    explicit RequestData(bool keepConnection);
    void addMessage(const MsgParams& msg);
    void addMessage(const MsgStdIn& msg);
    std::optional<Request> makeRequest();

    bool keepConnection() const;

private:
    std::string stdIn_;
    std::vector<std::pair<std::string, std::string>> params_;
    bool keepConnection_ = true;
    bool usedInRequest_ = false;
};

} //namespace fcgi
