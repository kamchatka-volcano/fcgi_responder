#pragma once
#include "errors.h"
#include <cstdint>
#include <string>

namespace fcgi {

enum class RecordType : std::uint8_t {
    BeginRequest = 1,
    AbortRequest = 2,
    EndRequest = 3,
    Params = 4,
    StdIn = 5,
    StdOut = 6,
    StdErr = 7,
    Data = 8,
    GetValues = 9,
    GetValuesResult = 10,
    UnknownType = 11,
};

inline RecordType recordTypeFromInt(std::uint8_t val)
{
    if (val < static_cast<std::uint8_t>(RecordType::BeginRequest) || val > static_cast<std::uint8_t>(RecordType::UnknownType))
        throw InvalidValue(InvalidValueType::RecordType, val);
    else
        return static_cast<RecordType>(val);
}

enum class Role : std::uint16_t {
    Responder = 1,
    Authorizer = 2,
    Filter = 3,
};

inline Role roleFromInt(std::uint16_t val)
{
    if (val < static_cast<std::uint16_t>(Role::Responder) || val > static_cast<std::uint16_t>(Role::Filter))
        throw InvalidValue(InvalidValueType::Role, val);
    else
        return static_cast<Role>(val);
}

enum class ResultConnectionState {
    Close = 0,
    KeepOpen = 1
};

enum class ProtocolStatus : std::uint8_t {
    RequestComplete = 0,
    CantMpxConn = 1,
    Overloaded = 2,
    UnknownRole = 3,
};

inline ProtocolStatus protocolStatusFromInt(std::uint8_t val)
{
    if (val > static_cast<std::uint8_t>(ProtocolStatus::UnknownRole))
        throw InvalidValue(InvalidValueType::ProtocolStatus, val);
    else
        return static_cast<ProtocolStatus>(val);
}

enum class ValueRequest {
    MaxConns = 0,
    MaxReqs = 1,
    MpxsConns = 2,
};

inline std::string valueRequestToString(ValueRequest request)
{
    switch (request) {
    case ValueRequest::MaxConns:
        return "FCGI_MAX_CONNS";
    case ValueRequest::MaxReqs:
        return "FCGI_MAX_REQS";
    case ValueRequest::MpxsConns:
        return "FCGI_MPXS_CONNS";
    }
    return {};
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
        throw InvalidValue(InvalidValueType::ValueRequest, request);
}

} //namespace fcgi
