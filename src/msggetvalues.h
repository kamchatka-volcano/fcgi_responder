#pragma once
#include "message.h"
#include "types.h"
#include <vector>

namespace fcgi{

class MsgGetValues : public Message{
public:
    MsgGetValues();
    void requestValue(ValueRequest request);
    const std::vector<ValueRequest>& requestList() const;

private:
    void toStream(std::ostream& output) const;
    void fromStream(std::istream& input);

private:
    std::vector<ValueRequest> valueRequestList_;
};

}
