#pragma once
#include "message.h"
#include "types.h"

namespace fcgi{

class MsgEndRequest : public Message{
public:
    MsgEndRequest();
    MsgEndRequest(uint32_t appStatus, ProtocolStatus protocolStatus);
    std::size_t size() const override;
    uint32_t appStatus() const;
    ProtocolStatus protocolStatus() const;
    bool operator==(const MsgEndRequest& other) const;

private:
    void toStream(std::ostream&) const override;
    void fromStream(std::istream&, std::size_t) override;

private:
    uint32_t appStatus_;
    ProtocolStatus protocolStatus_;

};

}
