#pragma once
#include "streamdatamessage.h"
#include <vector>
#include <string>
#include <unordered_map>

namespace fcgi{
class Request;
class MsgParams;

class RequestRegistry{
    struct RequestData{
        std::string stdIn;
        std::vector<std::pair<std::string, std::string>> params;
        bool movedToRequest = false;
    };

public:
    void registerRequest(uint16_t requestId);
    void fillRequestData(uint16_t requestId, const fcgi::MsgParams& msg);
    void fillRequestData(uint16_t requestId, const fcgi::MsgStdIn& msg);
    Request makeRequest(uint16_t requestId);
    void unregisterRequest(uint16_t requestId);

    bool hasRequest(uint16_t requestId) const;
    bool isEmpty() const;
    int requestCount() const;

private:
    std::unordered_map<uint16_t, RequestData> requestDataMap_;
};

}



