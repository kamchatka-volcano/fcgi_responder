#include "msgunknowntype.h"
#include "encoder.h"
#include "decoder.h"
#include <array>

namespace fcgi{

MsgUnknownType::MsgUnknownType(uint8_t unknownTypeValue)
    : unknownTypeValue_(unknownTypeValue)
{
}

std::size_t MsgUnknownType::size()
{
    return 8;
}

uint8_t MsgUnknownType::unknownTypeValue() const
{
    return unknownTypeValue_;
}

void MsgUnknownType::toStream(std::ostream &output) const
{
    auto encoder = Encoder(output);
    encoder << unknownTypeValue_;
    encoder.addPadding(7); //reserved bytes
}

void MsgUnknownType::fromStream(std::istream &input, std::size_t)
{
    auto decoder = Decoder(input);
    decoder >> unknownTypeValue_;
    decoder.skip(7); //reserved bytes
}

bool operator==(const MsgUnknownType &lhs, const MsgUnknownType &rhs)
{
    return lhs.unknownTypeValue_ == rhs.unknownTypeValue_;
}

}
