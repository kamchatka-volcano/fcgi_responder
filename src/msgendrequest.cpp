#include "msgendrequest.h"
#include "encoder.h"
#include "decoder.h"
#include <array>

using namespace fcgi;

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

void MsgEndRequest::fromStream(std::istream& input)
{
    auto protocolStatus = uint8_t{};
    auto decoder = Decoder(input);
    decoder >> appStatus_
            >> protocolStatus;
    decoder.skip(3); //reserved bytes

    protocolStatus_ = protocolStatusFromInt(protocolStatus);
}

