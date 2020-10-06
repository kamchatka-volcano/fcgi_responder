#include "decoder.h"
#include <algorithm>

using namespace fcgi;

Decoder::Decoder(std::istream& input)
    : input_(input)
{
}

Decoder& Decoder::operator>>(uint8_t& val)
{
    input_.read(reinterpret_cast<char*>(&val), 1);
    return *this;
}

Decoder& Decoder::operator>>(uint16_t& val)
{
    auto valIt = reinterpret_cast<char*>(&val);
    input_.read(valIt, 2);
#ifndef BIG_ENDIAN_HOST
    std::reverse(valIt, valIt + 2);
#endif
    return *this;
}

Decoder& Decoder::operator>>(uint32_t& val)
{
    auto valIt = reinterpret_cast<char*>(&val);
    input_.read(valIt, 4);
#ifndef BIG_ENDIAN_HOST
    std::reverse(valIt, valIt + 4);
#endif
    return *this;
}

Decoder& Decoder::operator>>(std::string& val)
{
    input_.read(&val[0], static_cast<std::streamsize>(val.size()));
    return *this;
}

void Decoder::skip(std::size_t numOfBytes)
{
    auto padding = std::string(numOfBytes, '0');
    input_.read(&padding[0], static_cast<std::streamsize>(padding.size()));
}
