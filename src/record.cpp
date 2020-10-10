#include "record.h"
#include "message.h"
#include "constants.h"
#include "errors.h"
#include "encoder.h"
#include "decoder.h"
#include "msgbeginrequest.h"
#include "msgendrequest.h"
#include "msggetvalues.h"
#include "msggetvaluesresult.h"
#include "msgparams.h"
#include "msgunknowntype.h"
#include "msgabortrequest.h"
#include "streamdatamessage.h"
#include <sstream>
#include <string>

using namespace fcgi;

Record::Record()
    : type_(RecordType::UnknownType)
    , requestId_(0)
    , message_(Message::createMessage(type_))
{}

Record::Record(RecordType type, uint16_t requestId)
    : type_(type)
    , requestId_(requestId)
    , message_(Message::createMessage(type_))
{
}

Record::Record(std::unique_ptr<Message> msg, uint16_t requestId)
    : type_(msg->recordType())
    , requestId_(requestId)
    , message_(std::move(msg))
{
}

Record::Record(Record&& record) = default;
Record::~Record() = default;

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
    return cHeaderSize + message_->size() + calcPaddingLength();
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

std::size_t Record::fromStream(std::istream& input, std::size_t inputSize)
{
    auto result = std::size_t{};
    try{
        result = read(input, inputSize);
    }
    catch(const ProtocolError&){throw;}
    catch(...){
        throw RecordReadError{};
    }
    return result;
}

void Record::write(std::ostream &output) const
{
    auto contentLength = static_cast<uint16_t>(message_->size());
    auto paddingLength = calcPaddingLength();
    auto reservedByte = uint8_t{};

    auto encoder = Encoder(output);
    encoder << cProtocolVersion
            << static_cast<uint8_t>(type_)
            << requestId_
            << contentLength
            << paddingLength
            << reservedByte;
    message_->write(output);
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

    if (inputSize - cHeaderSize < contentLength + paddingLength)
        return 0;

    type_ = recordTypeFromInt(type);
    if (type_ == RecordType::Invalid){
        decoder.skip(contentLength);
        decoder.skip(paddingLength);
        throw InvalidRecordType(type);
    }

    message_ = Message::createMessage(type_);
    message_->read(input, contentLength);
    decoder.skip(paddingLength);

    return cHeaderSize + contentLength + paddingLength;
}

RecordType Record::messageType(const Message& msg) const
{
    return msg.recordType();
}

uint8_t Record::calcPaddingLength() const
{
    auto result = static_cast<uint8_t>(8u - static_cast<uint8_t>((message_->size()) % 8));
    if (result == 8u)
        result = 0;
    return result;
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
    default: return false;
    }
}

}

bool Record::operator==(const Record& other) const
{
    return type_ == other.type_ &&
           requestId_ == other.requestId_ &&
           compareMessages(*this, other);
}
