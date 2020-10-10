#pragma once
#include "message.h"

namespace fcgi{

class MsgUnknownType : public Message{
public:
    MsgUnknownType();
    MsgUnknownType(uint8_t unknownTypeValue);
    std::size_t size() const override;
    uint8_t unknownTypeValue() const;
    bool operator==(const MsgUnknownType& other) const;

private:
    void toStream(std::ostream& output) const override;
    void fromStream(std::istream& input, std::size_t inputSize) override;

private:
    uint8_t unknownTypeValue_;
};

}
