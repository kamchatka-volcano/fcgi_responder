#pragma once
#include "message.h"

namespace fcgi{

class MsgAbortRequest : public Message<MsgAbortRequest>{
    friend class Message<MsgAbortRequest>;

public:
    MsgAbortRequest();
    bool operator==(const MsgAbortRequest& other) const;
    std::size_t size() const;

private:
    void toStream(std::ostream& output) const;
    void fromStream(std::istream& input, std::size_t inputSize);
};

}
