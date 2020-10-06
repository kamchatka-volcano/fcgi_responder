#pragma once
#include "message.h"

namespace fcgi{

class MsgUnknownType : public Message{
public:
    MsgUnknownType();
    MsgUnknownType(uint8_t unknownTypeValue);
    uint8_t unknownTypeValue() const;

private:
    void toStream(std::ostream& output) const override;
    void fromStream(std::istream& input) override;

private:
    uint8_t unknownTypeValue_;
};

}
