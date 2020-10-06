#include "record.h"
#include "message.h"
#include "constants.h"
#include "errors.h"
#include "encoder.h"
#include "decoder.h"
#include <sstream>
#include <string>

using namespace fcgi;

Record::Record()
    : type_(RecordType::UnknownType)
    , requestId_(0)
{}

Record::Record(RecordType type, uint16_t requestId)
    : type_(type)
    , requestId_(requestId)
{
}

Record::Record(const Message& msg, uint16_t requestId)
    : type_(msg.recordType())
    , requestId_(requestId)
{
    auto msgStream = std::ostringstream{};
    msg.write(msgStream);
    messageData_ = msgStream.str();
}

RecordType Record::type() const
{
    return type_;
}

uint16_t Record::requestId() const
{
    return requestId_;
}

const std::string& Record::messageData() const
{
    return messageData_;
}

void Record::toStream(std::ostream& output) const
{
    try{
        write(output);
    }
    catch(...){
        throw RecordWriteError{};
    }
}

int Record::fromStream(std::istream& input)
{
    auto result = 0;
    try{
        result = read(input);
    }
    catch(const ProtocolError&){throw;}
    catch(...){
        throw RecordReadError{};
    }
    return result;
}

void Record::write(std::ostream &output) const
{
    auto contentLength = static_cast<uint16_t>(messageData_.size());
    auto paddingLength = static_cast<uint8_t>(8u - static_cast<uint8_t>((contentLength) % 8));
    if (paddingLength == 8u)
        paddingLength = 0;
    auto reservedByte = uint8_t{};

    auto encoder = Encoder(output);
    encoder << cProtocolVersion
            << static_cast<uint8_t>(type_)
            << requestId_
            << contentLength
            << paddingLength
            << reservedByte
            << messageData_;
    encoder.addPadding(paddingLength);
}

int Record::read(std::istream &input)
{
    input.exceptions( std::istream::failbit | std::istream::badbit);
    auto beginPos = input.tellg();
    input.seekg(0, input.end);
    auto inputSize = input.tellg() - beginPos;
    input.seekg(beginPos);

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

    if (inputSize - cHeaderSize < contentLength + paddingLength)
        return 0;

    type_ = recordTypeFromInt(type);
    messageData_.resize(contentLength);
    decoder >> messageData_;
    decoder.skip(paddingLength);

    if (type_ == RecordType::Invalid)
        throw InvalidRecordType(type);

    auto readedByteCount = input.tellg() - beginPos;
    return static_cast<int>(readedByteCount);
}
