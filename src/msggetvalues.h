#pragma once
#include "message.h"
#include "types.h"
#include <vector>

namespace fcgi{

class MsgGetValues : public Message<MsgGetValues>{
    friend class Message<MsgGetValues>;

public:
    MsgGetValues();    
    const std::vector<ValueRequest>& requestList() const;
    void requestValue(ValueRequest request);
    bool operator==(const MsgGetValues& other) const;
    std::size_t size() const;

private:
    void toStream(std::ostream& output) const;
    void fromStream(std::istream& input, std::size_t inputSize);

private:
    std::vector<ValueRequest> valueRequestList_;
};

}
