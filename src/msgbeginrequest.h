#pragma once
#include "message.h"
#include "types.h"

namespace fcgi{

class MsgBeginRequest : public Message{
public:
    MsgBeginRequest();
    MsgBeginRequest(Role role, ResultConnectionState connectionState);
    Role role() const;
    ResultConnectionState resultConnectionState() const;

private:
    void toStream(std::ostream& output) const override;
    void fromStream(std::istream& input) override;


private:
    Role role_;
    ResultConnectionState resultConectionState_;
};

}

