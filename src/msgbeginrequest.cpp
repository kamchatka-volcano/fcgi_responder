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
}

void MsgBeginRequest::fromStream(std::istream &input)
{
    auto role = uint16_t{};
    auto flags = uint8_t{};
    auto decoder = Decoder(input);
    decoder >> role
            >> flags;
    role_ = roleFromInt(role);
    resultConectionState_ = static_cast<ResultConnectionState>(flags & cKeepConnectionMask);
}

