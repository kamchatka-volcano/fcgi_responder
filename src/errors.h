#pragma once
#include <cstdint>
#include <stdexcept>
#include <variant>

namespace fcgi {

class ProtocolError : public std::runtime_error {
public:
    explicit ProtocolError(const std::string&);
};

class UnsupportedVersion : public ProtocolError {
public:
    explicit UnsupportedVersion(std::uint8_t protocolVersion);
    std::uint8_t protocolVersion() const;

private:
    std::uint8_t protocolVersion_;
};

enum class InvalidValueType {
    RecordType,
    Role,
    ProtocolStatus,
    ValueRequest
};

class InvalidValue : public ProtocolError {
public:
    InvalidValue(InvalidValueType type, std::uint32_t value);
    InvalidValue(InvalidValueType type, const std::string& value);
    InvalidValueType type() const;
    std::uint32_t asInt() const;
    std::string asString() const;

    const char* what() const noexcept override;

private:
    InvalidValueType type_;
    std::variant<std::uint32_t, std::string> value_;
    std::string msg_;
};

class RecordMessageReadError : public ProtocolError {
public:
    RecordMessageReadError(const std::string&, std::size_t recordSize);
    std::size_t recordSize() const;

private:
    std::size_t recordSize_;
};

class InvalidRecordType : public ProtocolError {
public:
    explicit InvalidRecordType(std::uint8_t recordType);
    std::uint8_t recordType() const;

private:
    std::uint8_t typeValue_;
};

} //namespace fcgi
