#include "errors.h"

namespace fcgi{

ProtocolError::ProtocolError(const std::string& msg)
    : std::runtime_error(msg)
{
}

UnrecoverableProtocolError::UnrecoverableProtocolError(const std::string& msg)
    : std::runtime_error(msg)
{
}

UnsupportedVersion::UnsupportedVersion(uint8_t protocolVersion)
    : ProtocolError("Protocol version \"" + std::to_string(protocolVersion) + "\" isn't supported.")
    , protocolVersion_(protocolVersion)
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
    : ProtocolError("")
    , type_(type)
    , value_(value)
    , msg_(invalidValueTypeToString(type_) + " value \"" + asString() + "\" is invalid.")
{
}

InvalidValue::InvalidValue(InvalidValueType type, const std::string& value)
    : ProtocolError("")
    , type_(type)
    , value_(0)
    , valueStr_(value)
    , msg_(invalidValueTypeToString(type_) + " value \"" + asString() + "\" is invalid.")
{
}

InvalidValueType InvalidValue::type() const
{
    return type_;
}

uint32_t InvalidValue::asInt() const
{
    return value_;
}

std::string InvalidValue::asString() const
{
    if (type_ == InvalidValueType::ValueRequest)
        return valueStr_;
    else
        return std::to_string(value_);
}

const char* InvalidValue::what() const noexcept
{
    return msg_.c_str();
}

RecordReadError::RecordReadError(const std::string& msg, std::size_t recordSize)
    : ProtocolError(msg)
    , recordSize_(recordSize)
{
}

std::size_t RecordReadError::recordSize() const
{
    return recordSize_;
}

InvalidRecordType::InvalidRecordType(uint8_t typeValue, std::size_t recordSize)
    : RecordReadError("Record type \"" + std::to_string(typeValue) + "\" is invalid.", recordSize)
    , typeValue_(typeValue)
{}

uint8_t InvalidRecordType::recordType() const
{
    return typeValue_;
}

}
