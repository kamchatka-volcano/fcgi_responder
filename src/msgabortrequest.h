#pragma once
#include "types.h"
#include <ostream>
#include <istream>

namespace fcgi{

class MsgAbortRequest{
public:
    static const RecordType recordType = RecordType::AbortRequest;

public:
    static std::size_t size();

    void toStream(std::ostream& output) const;
    void fromStream(std::istream& input, std::size_t inputSize);
};

bool operator==(const MsgAbortRequest& lhs, const MsgAbortRequest& rhs);

}
