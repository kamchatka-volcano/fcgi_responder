#pragma once
#include <stdexcept>

namespace fcgi{

class ProtocolError : public std::runtime_error{
public:
    ProtocolError(const std::string&);
};

class UnsupportedVersion : public ProtocolError{
public:
    UnsupportedVersion(uint8_t protocolVersion);
    uint8_t protocolVersion() const;

private:
    uint8_t protocolVersion_;
};

class InvalidRecordType : public ProtocolError{
public:
    InvalidRecordType(uint8_t recordType);
    uint8_t recordType() const;

private:
    uint8_t recordType_;
};

class StreamError : public std::runtime_error{
public:
    StreamError(const std::string&);
};

class MessageReadError : public StreamError{
public:
    MessageReadError();
};

class MessageWriteError : public StreamError{
public:
    MessageWriteError();
};

class RecordReadError : public StreamError{
public:
    RecordReadError();
};

class RecordWriteError : public StreamError{
public:
    RecordWriteError();
};

}

