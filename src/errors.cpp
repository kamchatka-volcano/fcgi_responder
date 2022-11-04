#include "errors.h"
#include <string>

namespace fcgi{

ProtocolError::ProtocolError(const std::string& msg)
    : std::runtime_error{msg}
{
}

UnsupportedVersion::UnsupportedVersion(uint8_t protocolVersion)
    : ProtocolError{"Protocol version \"" + std::to_string(protocolVersion) + "\" isn't supported."}
    , protocolVersion_{protocolVersion}
{
}

uint8_t UnsupportedVersion::protocolVersion() const
{
    return protocolVersion_;
}

namespace
{

std::string invalidValueTypeToString(InvalidValueType type)
{
    switch (type){
    case InvalidValueType::RecordType: return "Record type";
    case InvalidValueType::Role: return "Role";
    case InvalidValueType::ProtocolStatus: return "Protocol status";
    case InvalidValueType::ValueRequest: return "Value request";
    }
    return {};
}

}

InvalidValue::InvalidValue(InvalidValueType type, uint32_t value)
    : ProtocolError{""}
    , type_{type}
    , value_{value}
    , msg_{invalidValueTypeToString(type_) + " value \"" + asString() + "\" is invalid."}
{
}

InvalidValue::InvalidValue(InvalidValueType type, const std::string& value)
    : ProtocolError{""}
    , type_{type}
    , value_{value}
    , msg_{invalidValueTypeToString(type_) + " value \"" + asString() + "\" is invalid."}
{
}

InvalidValueType InvalidValue::type() const
{
    return type_;
}

uint32_t InvalidValue::asInt() const
{
    return std::get<uint32_t>(value_);
}

std::string InvalidValue::asString() const
{
    if (std::holds_alternative<std::string>(value_))
        return std::get<std::string>(value_);
    else
        return std::to_string(std::get<uint32_t>(value_));
}

const char* InvalidValue::what() const noexcept
{
    return msg_.c_str();
}

RecordMessageReadError::RecordMessageReadError(const std::string& msg, std::size_t recordSize)
    : ProtocolError{"Record message read error: " + msg}
    , recordSize_{recordSize}
{}

std::size_t RecordMessageReadError::recordSize() const
{
    return recordSize_;
}

InvalidRecordType::InvalidRecordType(uint8_t typeValue)
    : ProtocolError{"Record type \"" + std::to_string(typeValue) + "\" is invalid."}
    , typeValue_{typeValue}
{}

uint8_t InvalidRecordType::recordType() const
{
    return typeValue_;
}

}
