#pragma once
#include "message.h"

namespace fcgi{

class MsgAbortRequest : public Message
{
public:
    MsgAbortRequest();
    std::size_t size() const override;
    bool operator==(const MsgAbortRequest& other) const;

private:
    void toStream(std::ostream& output) const override;
    void fromStream(std::istream& input, std::size_t inputSize) override;
};

}
