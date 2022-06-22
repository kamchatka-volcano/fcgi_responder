#pragma once
#include "types.h"
#include <istream>
#include <ostream>

namespace fcgi{

class MsgBeginRequest{
public:
    static const RecordType recordType = RecordType::BeginRequest;

public:
    MsgBeginRequest();
    MsgBeginRequest(Role role, ResultConnectionState connectionState);
    Role role() const;
    ResultConnectionState resultConnectionState() const;
    static std::size_t size();

    void toStream(std::ostream& output) const;
    void fromStream(std::istream& input, std::size_t inputSize);

private:
    friend bool operator==(const MsgBeginRequest& lhs, const MsgBeginRequest& rhs);

private:
    Role role_;
    ResultConnectionState resultConnectionState_;
};

bool operator==(const MsgBeginRequest& lhs, const MsgBeginRequest& rhs);

}

