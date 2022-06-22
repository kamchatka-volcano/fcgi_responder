#include "msgendrequest.h"
#include "encoder.h"
#include "decoder.h"
#include <array>

namespace fcgi{

MsgEndRequest::MsgEndRequest()
    : appStatus_(0)
    , protocolStatus_(ProtocolStatus::UnknownRole)
{
}

MsgEndRequest::MsgEndRequest(uint32_t appStatus, ProtocolStatus protocolStatus)
    : appStatus_(appStatus)
    , protocolStatus_(protocolStatus)
{
}

std::size_t MsgEndRequest::size()
{
    return 8;
}

uint32_t MsgEndRequest::appStatus() const
{
    return appStatus_;
}

ProtocolStatus MsgEndRequest::protocolStatus() const
{
    return protocolStatus_;
}

void MsgEndRequest::toStream(std::ostream& output) const
{
    auto encoder = Encoder(output);
    encoder << appStatus_
            << static_cast<uint8_t>(protocolStatus_);
    encoder.addPadding(3); //reserved bytes
}

void MsgEndRequest::fromStream(std::istream& input, std::size_t)
{
    auto protocolStatus = uint8_t{};
    auto decoder = Decoder(input);
    decoder >> appStatus_
            >> protocolStatus;
    decoder.skip(3); //reserved bytes

    protocolStatus_ = protocolStatusFromInt(protocolStatus);
}

bool operator==(const MsgEndRequest& lhs, const MsgEndRequest& rhs)
{
    return lhs.appStatus_ == rhs.appStatus_ &&
           lhs.protocolStatus_ == rhs.protocolStatus_;
}

}
