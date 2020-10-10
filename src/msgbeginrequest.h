#pragma once
#include "message.h"
#include "types.h"

namespace fcgi{

class MsgBeginRequest : public Message{
public:
    MsgBeginRequest();
    MsgBeginRequest(Role role, ResultConnectionState connectionState);
    std::size_t size() const override;
    Role role() const;
    ResultConnectionState resultConnectionState() const;
    bool operator==(const MsgBeginRequest& other) const;

private:
    void toStream(std::ostream& output) const override;
    void fromStream(std::istream& input, std::size_t inputSize) override;


private:
    Role role_;
    ResultConnectionState resultConectionState_;
};

}

