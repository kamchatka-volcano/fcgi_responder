#include "msgbeginrequest.h"
#include "constants.h"
#include "decoder.h"
#include "encoder.h"
#include "types.h"

namespace fcgi {

MsgBeginRequest::MsgBeginRequest(Role role, ResultConnectionState connectionState)
    : role_{role}
    , resultConnectionState_{connectionState}
{
}

std::size_t MsgBeginRequest::size()
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

void MsgBeginRequest::toStream(std::ostream& output) const
{
    auto encoder = Encoder(output);
    encoder << static_cast<uint16_t>(role_) << static_cast<uint8_t>(resultConnectionState_);
    encoder.addPadding(5); //reserved bytes
}

void MsgBeginRequest::fromStream(std::istream& input, std::size_t)
{
    auto role = uint16_t{};
    auto flags = uint8_t{};
    auto decoder = Decoder(input);
    decoder >> role >> flags;
    decoder.skip(5); //reservedBytes
    role_ = roleFromInt(role);
    resultConnectionState_ = static_cast<ResultConnectionState>(flags & hardcoded::keepConnectionMask);
}

bool operator==(const MsgBeginRequest& lhs, const MsgBeginRequest& rhs)
{
    return lhs.role_ == rhs.role_ && lhs.resultConnectionState_ == rhs.resultConnectionState_;
}

} //namespace fcgi
