#include "record.h"
#include "message.h"
#include "constants.h"
#include "errors.h"
#include "encoder.h"
#include "decoder.h"
#include <sstream>
#include <string>

namespace fcgi{

Record::Record()
    : type_(RecordType::UnknownType)
    , requestId_(0)
{
    initMessage();
}

Record::Record(RecordType type, uint16_t requestId)
    : type_(type)
    , requestId_(requestId)
{
    initMessage();
}

RecordType Record::type() const
{
    return type_;
}

uint16_t Record::requestId() const
{
    return requestId_;
}

std::size_t Record::size() const
{
    return cHeaderSize + messageSize() + calcPaddingLength();
}

void Record::toStream(std::ostream& output) const
{
    write(output);
}

std::size_t Record::fromStream(std::istream& input, std::size_t inputSize)
{
    auto result = std::size_t{};
    result = read(input, inputSize);
    return result;
}

void Record::write(std::ostream &output) const
{
    auto contentLength = static_cast<uint16_t>(messageSize());
    auto paddingLength = calcPaddingLength();
    auto reservedByte = uint8_t{};

    auto encoder = Encoder(output);
    encoder << cProtocolVersion
            << static_cast<uint8_t>(type_)
            << requestId_
            << contentLength
            << paddingLength
            << reservedByte;
    writeMessage(output);
    encoder.addPadding(paddingLength);
}

std::size_t Record::read(std::istream &input, std::size_t inputSize)
{
    input.exceptions( std::istream::failbit | std::istream::badbit);
    if (inputSize < cHeaderSize)
        return 0;

    auto decoder = Decoder(input);
    auto protocolVersion = uint8_t{};
    decoder >> protocolVersion;
    if (protocolVersion != cProtocolVersion)
        throw UnsupportedVersion(protocolVersion);

    auto type = uint8_t{};
    auto contentLength = uint16_t{};
    auto paddingLength = uint8_t{};
    auto reservedByte = uint8_t{};
    decoder >> type
            >> requestId_
            >> contentLength
            >> paddingLength
            >> reservedByte;

    auto recordSize = static_cast<std::size_t>(cHeaderSize + contentLength + paddingLength);

    if (inputSize < recordSize)
        return 0;

    try{
        type_ = recordTypeFromInt(type);
    }
    catch(const InvalidValue& e){
        throw InvalidRecordType(static_cast<uint8_t>(e.asInt()), recordSize);
    }

    try{
        initMessage();
        readMessage(input, contentLength);
        decoder.skip(paddingLength);
    }
    catch(ProtocolError& e){
        throw RecordReadError(e.what(), recordSize);
    }

    return recordSize;
}

uint8_t Record::calcPaddingLength() const
{
    auto result = static_cast<uint8_t>(8u - static_cast<uint8_t>(messageSize() % 8));
    if (result == 8u)
        result = 0;
    return result;
}

void Record::initMessage()
{
    switch(type_){
    case RecordType::BeginRequest: message_.emplace<MsgBeginRequest>(); break;
    case RecordType::AbortRequest: message_.emplace<MsgAbortRequest>(); break;
    case RecordType::EndRequest: message_.emplace<MsgEndRequest>(); break;
    case RecordType::Params: message_.emplace<MsgParams>(); break;
    case RecordType::StdIn: message_.emplace<MsgStdIn>(); break;
    case RecordType::StdOut: message_.emplace<MsgStdOut>(); break;
    case RecordType::StdErr: message_.emplace<MsgStdErr>(); break;
    case RecordType::Data: message_.emplace<MsgData>(); break;
    case RecordType::GetValues: message_.emplace<MsgGetValues>(); break;
    case RecordType::GetValuesResult: message_.emplace<MsgGetValuesResult>(); break;
    case RecordType::UnknownType: message_.emplace<MsgUnknownType>(); break;
    }
}

std::size_t Record::messageSize() const
{
    return std::visit([](auto&& msg){return msg.size();}, message_);
}

void Record::readMessage(std::istream &input, std::size_t inputSize)
{
    std::visit([&](auto&& msg){msg.read(input, inputSize);}, message_);
}

void Record::writeMessage(std::ostream &output) const
{
    std::visit([&](auto&& msg){msg.write(output);}, message_);
}

namespace  {

bool compareMessages(const fcgi::Record& lhs, const fcgi::Record& rhs)
{
    switch(lhs.type()){
    case fcgi::RecordType::Data: return lhs.getMessage<fcgi::MsgData>() == rhs.getMessage<fcgi::MsgData>();
    case fcgi::RecordType::StdIn: return lhs.getMessage<fcgi::MsgStdIn>() == rhs.getMessage<fcgi::MsgStdIn>();
    case fcgi::RecordType::StdOut: return lhs.getMessage<fcgi::MsgStdOut>() == rhs.getMessage<fcgi::MsgStdOut>();
    case fcgi::RecordType::StdErr: return lhs.getMessage<fcgi::MsgStdErr>() == rhs.getMessage<fcgi::MsgStdErr>();
    case fcgi::RecordType::Params: return lhs.getMessage<fcgi::MsgParams>() == rhs.getMessage<fcgi::MsgParams>();
    case fcgi::RecordType::GetValues: return lhs.getMessage<fcgi::MsgGetValues>() == rhs.getMessage<fcgi::MsgGetValues>();
    case fcgi::RecordType::GetValuesResult: return lhs.getMessage<fcgi::MsgGetValuesResult>() == rhs.getMessage<fcgi::MsgGetValuesResult>();
    case fcgi::RecordType::BeginRequest: return lhs.getMessage<fcgi::MsgBeginRequest>() == rhs.getMessage<fcgi::MsgBeginRequest>();
    case fcgi::RecordType::EndRequest: return lhs.getMessage<fcgi::MsgEndRequest>() == rhs.getMessage<fcgi::MsgEndRequest>();
    case fcgi::RecordType::AbortRequest: return lhs.getMessage<fcgi::MsgAbortRequest>() == rhs.getMessage<fcgi::MsgAbortRequest>();
    case fcgi::RecordType::UnknownType: return lhs.getMessage<fcgi::MsgUnknownType>() == rhs.getMessage<fcgi::MsgUnknownType>();
    }
    return false;
}

}

bool Record::operator==(const Record& other) const
{
    return type_ == other.type_ &&
           requestId_ == other.requestId_ &&
           compareMessages(*this, other);
}

}
