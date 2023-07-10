#pragma once
#include "types.h"
#include <istream>
#include <ostream>

namespace fcgi {

class MsgEndRequest {
public:
    static const RecordType recordType = RecordType::EndRequest;

public:
    MsgEndRequest() = default;
    MsgEndRequest(uint32_t appStatus, ProtocolStatus protocolStatus);
    uint32_t appStatus() const;
    ProtocolStatus protocolStatus() const;
    static std::size_t size();

    void toStream(std::ostream&) const;
    void fromStream(std::istream&, std::size_t);

private:
    friend bool operator==(const MsgEndRequest& lhs, const MsgEndRequest& rhs);

private:
    uint32_t appStatus_ = 0;
    ProtocolStatus protocolStatus_ = ProtocolStatus::UnknownRole;
};

bool operator==(const MsgEndRequest& lhs, const MsgEndRequest& rhs);

} //namespace fcgi
