#pragma once
#include "types.h"
#include <cstdint>
#include <istream>
#include <ostream>

namespace fcgi {

class MsgUnknownType {
public:
    static const RecordType recordType = RecordType::UnknownType;

public:
    MsgUnknownType() = default;
    explicit MsgUnknownType(std::uint8_t unknownTypeValue);
    std::uint8_t unknownTypeValue() const;
    static std::size_t size();

    void toStream(std::ostream& output) const;
    void fromStream(std::istream& input, std::size_t inputSize);

private:
    friend bool operator==(const MsgUnknownType& lhs, const MsgUnknownType& rhs);

private:
    std::uint8_t unknownTypeValue_ = 0;
};

bool operator==(const MsgUnknownType& lhs, const MsgUnknownType& rhs);

} //namespace fcgi
