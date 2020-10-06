#include "msgunknowntype.h"
#include "encoder.h"
#include "decoder.h"
#include <array>

using namespace fcgi;

MsgUnknownType::MsgUnknownType()
    : Message(RecordType::UnknownType)
{
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

void MsgUnknownType::fromStream(std::istream &input)
{
    auto decoder = Decoder(input);
    decoder >> unknownTypeValue_;
    decoder.skip(7); //reserved bytes
}

