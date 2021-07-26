#pragma once
#include "message.h"
#include "types.h"

namespace fcgi{

class MsgBeginRequest : public Message<MsgBeginRequest>{
    friend class Message<MsgBeginRequest>;

public:
    MsgBeginRequest();
    MsgBeginRequest(Role role, ResultConnectionState connectionState);
    Role role() const;
    ResultConnectionState resultConnectionState() const;
    bool operator==(const MsgBeginRequest& other) const;
    std::size_t size() const;

private:
    void toStream(std::ostream& output) const;
    void fromStream(std::istream& input, std::size_t inputSize);


private:
    Role role_;
    ResultConnectionState resultConnectionState_;
};

}

