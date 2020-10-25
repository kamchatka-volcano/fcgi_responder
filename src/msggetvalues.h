#pragma once
#include "message.h"
#include "types.h"
#include <vector>

namespace fcgi{

class MsgGetValues : public Message{
public:
    MsgGetValues();
    std::size_t size() const override;
    const std::vector<ValueRequest>& requestList() const;
    bool operator==(const MsgGetValues& other) const;

    void requestValue(ValueRequest request);

private:
    void toStream(std::ostream& output) const override;
    void fromStream(std::istream& input, std::size_t inputSize) override;

private:
    std::vector<ValueRequest> valueRequestList_;
};

}
