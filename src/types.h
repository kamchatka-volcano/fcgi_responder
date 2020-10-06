#pragma once
#include <string>
#include <cstdint>

namespace fcgi{

enum class RecordType : uint8_t{
    BeginRequest    = 1,
    AbortRequest    = 2,
    EndRequest      = 3,
    Params          = 4,
    StdIn           = 5,
    StdOut          = 6,
    StdErr          = 7,
    Data            = 8,
    GetValues       = 9,
    GetValuesResult = 10,
    UnknownType     = 11,
    Invalid
};

inline RecordType recordTypeFromInt(uint8_t val)
{
    if (val < static_cast<uint8_t>(RecordType::BeginRequest) || val > static_cast<uint8_t>(RecordType::UnknownType))
        return RecordType::Invalid;
    else
        return static_cast<RecordType>(val);
}

enum class Role : uint16_t{
    Responder  = 1,
    Authorizer = 2,
    Filter     = 3,
    Invalid
};

inline Role roleFromInt(uint16_t val)
{
    if (val < static_cast<uint16_t>(Role::Responder) || val > static_cast<uint16_t>(Role::Filter))
        return Role::Invalid;
    else
        return static_cast<Role>(val);
}

enum class ResultConnectionState{
    Close    = 0,
    KeepOpen = 1
};

enum class ProtocolStatus : uint8_t{
    RequestComplete = 0,
    CantMpxConn     = 1,
    Overloaded      = 2,
    UnknownRole     = 3,
    Invalid
};

inline ProtocolStatus protocolStatusFromInt(uint8_t val)
{
    if (val > static_cast<uint8_t>(ProtocolStatus::UnknownRole))
        return ProtocolStatus::Invalid;
    else
        return static_cast<ProtocolStatus>(val);
}

enum class ValueRequest{
    MaxConns  = 0,
    MaxReqs   = 1,
    MpxsConns = 2,
    Invalid
};

inline std::string valueRequestToString(ValueRequest request)
{
    switch(request){
    case ValueRequest::MaxConns:  return "FCGI_MAX_CONNS";
    case ValueRequest::MaxReqs:   return "FCGI_MAX_REQS";
    case ValueRequest::MpxsConns: return "FCGI_MPXS_CONNS";
    default: return {};
    }
}

inline ValueRequest valueRequestFromString(const std::string& request)
{
    if (request == "FCGI_MAX_CONNS")
        return ValueRequest::MaxConns;
    else if (request == "FCGI_MAX_REQS")
        return ValueRequest::MaxReqs;
    else if (request == "FCGI_MPXS_CONNS")
        return ValueRequest::MpxsConns;
    else
        return ValueRequest::Invalid;
}

}

