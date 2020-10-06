#pragma once
#include "message.h"
#include "types.h"

namespace fcgi{

class MsgEndRequest : public Message{
public:
    MsgEndRequest();
    MsgEndRequest(uint32_t appStatus, ProtocolStatus protocolStatus);
    uint32_t appStatus() const;
    ProtocolStatus protocolStatus() const;

private:
    void toStream(std::ostream&) const override;
    void fromStream(std::istream&) override;

private:
    uint32_t appStatus_;
    ProtocolStatus protocolStatus_;

};

}
