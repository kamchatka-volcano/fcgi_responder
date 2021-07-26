#include "msgbeginrequest.h"
#include "constants.h"
#include "types.h"
#include "encoder.h"
#include "decoder.h"

namespace fcgi{

MsgBeginRequest::MsgBeginRequest()
    : Message(RecordType::BeginRequest)
    , role_(Role::Responder)
    , resultConnectionState_(ResultConnectionState::Close)
{
}

MsgBeginRequest::MsgBeginRequest(Role role, ResultConnectionState connectionState)
    : Message(RecordType::BeginRequest)
    , role_(role)
    , resultConnectionState_(connectionState)
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
    return resultConnectionState_;
}

void MsgBeginRequest::toStream(std::ostream &output) const
{
    auto encoder = Encoder(output);
    encoder << static_cast<uint16_t>(role_)
            << static_cast<uint8_t>(resultConnectionState_);
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
    resultConnectionState_ = static_cast<ResultConnectionState>(flags & cKeepConnectionMask);
}

bool MsgBeginRequest::operator==(const MsgBeginRequest& other) const
{
    return role_ == other.role_ &&
           resultConnectionState_ == other.resultConnectionState_;
}

}
