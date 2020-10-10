#pragma once
#include "message.h"
#include "types.h"
#include <vector>

namespace fcgi{

class MsgGetValues : public Message{
public:
    MsgGetValues();
    std::size_t size() const override;
    void requestValue(ValueRequest request);
    const std::vector<ValueRequest>& requestList() const;
    bool operator==(const MsgGetValues& other) const;

private:
    void toStream(std::ostream& output) const;
    void fromStream(std::istream& input, std::size_t inputSize);

private:
    std::vector<ValueRequest> valueRequestList_;
};

}
