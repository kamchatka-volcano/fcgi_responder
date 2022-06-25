#include "encoder.h"
#include <algorithm>

namespace fcgi{

Encoder::Encoder(std::ostream& stream)
    : output_{stream}
{
}

Encoder& Encoder::operator<<(uint8_t val)
{
    output_.write(reinterpret_cast<const char*>(&val), 1);
    return *this;
}

Encoder& Encoder::operator<<(uint16_t val)
{
    auto valIt = reinterpret_cast<char*>(&val);
#ifndef BIG_ENDIAN_HOST
    std::reverse(valIt, valIt + 2);
#endif
    output_.write(valIt, 2);
    return *this;
}

Encoder& Encoder::operator<<(uint32_t val)
{
    auto valIt = reinterpret_cast<char*>(&val);
#ifndef BIG_ENDIAN_HOST
    std::reverse(valIt, valIt + 4);
#endif
    output_.write(valIt, 4);
    return *this;
}

Encoder& Encoder::operator<<(const std::string& val)
{
    output_.write(val.c_str(), static_cast<std::streamsize>(val.size()));
    return *this;
}

void Encoder::addPadding(std::size_t numOfBytes)
{
    auto ch = char{};
    for (auto i = 0u; i < numOfBytes; ++i)
        output_.put(ch);
}

}
