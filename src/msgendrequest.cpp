#include "msgendrequest.h"
#include "encoder.h"
#include "decoder.h"
#include <array>

namespace fcgi{

MsgEndRequest::MsgEndRequest()
    : Message(RecordType::EndRequest)
{
}

MsgEndRequest::MsgEndRequest(uint32_t appStatus, ProtocolStatus protocolStatus)
    : Message (RecordType::EndRequest)
    , appStatus_(appStatus)
    , protocolStatus_(protocolStatus)
{
}

std::size_t MsgEndRequest::size() const
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

bool MsgEndRequest::operator==(const MsgEndRequest& other) const
{
    return appStatus_ == other.appStatus_ &&
           protocolStatus_ == other.protocolStatus_;
}

}
