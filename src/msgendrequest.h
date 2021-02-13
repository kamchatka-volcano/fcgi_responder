#pragma once
#include "message.h"
#include "types.h"

namespace fcgi{

class MsgEndRequest : public Message<MsgEndRequest>{
    friend class Message<MsgEndRequest>;

public:
    MsgEndRequest();
    MsgEndRequest(uint32_t appStatus, ProtocolStatus protocolStatus);
    uint32_t appStatus() const;
    ProtocolStatus protocolStatus() const;
    bool operator==(const MsgEndRequest& other) const;
    std::size_t size() const;

private:    
    void toStream(std::ostream&) const;
    void fromStream(std::istream&, std::size_t);

private:
    uint32_t appStatus_;
    ProtocolStatus protocolStatus_;

};

}
