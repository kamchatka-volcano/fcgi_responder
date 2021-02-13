#pragma once
#include "message.h"

namespace fcgi{

class MsgUnknownType : public Message<MsgUnknownType>{
    friend class Message<MsgUnknownType>;

public:
    MsgUnknownType();
    MsgUnknownType(uint8_t unknownTypeValue);
    std::size_t size() const;
    uint8_t unknownTypeValue() const;
    bool operator==(const MsgUnknownType& other) const;

private:
    void toStream(std::ostream& output) const;
    void fromStream(std::istream& input, std::size_t inputSize);

private:
    uint8_t unknownTypeValue_;
};

}
