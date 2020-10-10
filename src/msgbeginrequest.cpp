#include "msgbeginrequest.h"
#include "constants.h"
#include "types.h"
#include "encoder.h"
#include "decoder.h"

using namespace fcgi;

MsgBeginRequest::MsgBeginRequest()
    : Message(RecordType::BeginRequest)
{
}

MsgBeginRequest::MsgBeginRequest(Role role, ResultConnectionState connectionState)
    : Message(RecordType::BeginRequest)
    , role_(role)
    , resultConectionState_(connectionState)
{
}

std::size_t MsgBeginRequest::size() const
{
    return 8;
}

Role MsgBeginRequest::role() const
{
    return role_;
}

ResultConnectionState MsgBeginRequest::resultConnectionState() const
{
    return resultConectionState_;
}

void MsgBeginRequest::toStream(std::ostream &output) const
{
    auto encoder = Encoder(output);
    encoder << static_cast<uint16_t>(role_)
            << static_cast<uint8_t>(resultConectionState_);
    encoder.addPadding(5); //reserved bytes
}

void MsgBeginRequest::fromStream(std::istream &input, std::size_t)
{
    auto role = uint16_t{};
    auto flags = uint8_t{};
    auto decoder = Decoder(input);
    decoder >> role
            >> flags;
    decoder.skip(5); //reservedBytes
    role_ = roleFromInt(role);
    resultConectionState_ = static_cast<ResultConnectionState>(flags & cKeepConnectionMask);
}

bool MsgBeginRequest::operator==(const MsgBeginRequest& other) const
{
    return role_ == other.role_ &&
           resultConectionState_ == other.resultConectionState_;
}
