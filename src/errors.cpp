#include "errors.h"

using namespace fcgi;

ProtocolError::ProtocolError(const std::string& msg)
    : std::runtime_error(msg)
{
}

UnsupportedVersion::UnsupportedVersion(uint8_t protocolVersion)
    : ProtocolError("Protocol version" + std::to_string(protocolVersion) + "isn't supported.")
    , protocolVersion_(protocolVersion)
{
}

uint8_t UnsupportedVersion::protocolVersion() const
{
    return protocolVersion_;
}

InvalidRecordType::InvalidRecordType(uint8_t recordType)
    : ProtocolError("Record type " + std::to_string(recordType) + "is invalid.")
    , recordType_(recordType)
{
}

uint8_t InvalidRecordType::recordType() const
{
    return recordType_;
}

StreamError::StreamError(const std::string& msg)
    : std::runtime_error (msg)
{
}

MessageReadError::MessageReadError()
    : StreamError("Message read error")
{
}

MessageWriteError::MessageWriteError()
    : StreamError("Message write error")
{
}

RecordReadError::RecordReadError()
    : StreamError("Record read error")
{
}

RecordWriteError::RecordWriteError()
    : StreamError("Record write error")
{
}
