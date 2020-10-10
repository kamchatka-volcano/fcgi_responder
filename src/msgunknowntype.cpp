#include "msgunknowntype.h"
#include "encoder.h"
#include "decoder.h"
#include <array>

using namespace fcgi;

MsgUnknownType::MsgUnknownType()
    : Message(RecordType::UnknownType)
{
}

std::size_t MsgUnknownType::size() const
{
    return 8;
}

MsgUnknownType::MsgUnknownType(uint8_t unknownTypeValue)
    : Message(RecordType::UnknownType)
    , unknownTypeValue_(unknownTypeValue)
{
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

bool MsgUnknownType::operator==(const MsgUnknownType& other) const
{
    return unknownTypeValue_ == other.unknownTypeValue_;
}
