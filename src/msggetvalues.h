#pragma once
#include "types.h"
#include <istream>
#include <ostream>
#include <vector>

namespace fcgi {

class MsgGetValues {
public:
    static const RecordType recordType = RecordType::GetValues;

public:
    const std::vector<ValueRequest>& requestList() const;
    void requestValue(ValueRequest request);
    std::size_t size() const;

    void toStream(std::ostream& output) const;
    void fromStream(std::istream& input, std::size_t inputSize);

private:
    friend bool operator==(const MsgGetValues& lhs, const MsgGetValues& rhs);

private:
    std::vector<ValueRequest> valueRequestList_;
};

bool operator==(const MsgGetValues& lhs, const MsgGetValues& rhs);

} //namespace fcgi
