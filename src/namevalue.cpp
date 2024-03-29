#include "namevalue.h"
#include "decoder.h"
#include "encoder.h"
#include "errors.h"

namespace fcgi {

NameValue::NameValue(std::size_t maxSize)
    : maxSize_{maxSize}
{
}

std::size_t NameValue::size() const
{
    auto result = std::size_t{};
    if (name_.size() <= 127)
        result += 1;
    else
        result += 4;
    if (value_.size() <= 127)
        result += 1;
    else
        result += 4;
    result += name_.size();
    result += value_.size();

    return result;
}

const std::string& NameValue::name() const
{
    return name_;
}

const std::string& NameValue::value() const
{
    return value_;
}

void NameValue::setName(const std::string& name)
{
    name_ = name;
}

void NameValue::setValue(const std::string& value)
{
    value_ = value;
}

namespace {

std::uint32_t readLengthFromStream(std::istream& input)
{
    auto ch = char{};
    input.get(ch);

    auto length = 0;
    if ((ch >> 7) == 0)
        length = ch;
    else {
        auto lengthB3 = static_cast<unsigned char>(ch);
        input.get(ch);
        auto lengthB2 = static_cast<unsigned char>(ch);
        input.get(ch);
        auto lengthB1 = static_cast<unsigned char>(ch);
        input.get(ch);
        auto lengthB0 = static_cast<unsigned char>(ch);

        length = ((lengthB3 & 0x7f) << 24) + (lengthB2 << 16) + (lengthB1 << 8) + lengthB0;
    }
    return static_cast<std::uint32_t>(length);
}

void writeLengthToStream(std::uint32_t length, std::ostream& output)
{
    auto encoder = Encoder(output);
    if (length <= 127) {
        encoder << static_cast<std::uint8_t>(length);
    }
    else {
        length |= 0x80000000;
        encoder << length;
    }
}

} //namespace

void NameValue::toStream(std::ostream& output) const
{
    writeLengthToStream(static_cast<std::uint32_t>(name_.size()), output);
    writeLengthToStream(static_cast<std::uint32_t>(value_.size()), output);
    auto encoder = Encoder(output);
    encoder << name_ << value_;
}

void NameValue::fromStream(std::istream& input)
{
    auto nameLength = readLengthFromStream(input);
    auto valueLength = readLengthFromStream(input);
    if (nameLength + valueLength > maxSize_)
        throw ProtocolError("Name and value length exceeds max size");

    name_.resize(nameLength);
    value_.resize(valueLength);
    auto decoder = Decoder(input);
    decoder >> name_ >> value_;
}

bool operator==(const NameValue& lhs, const NameValue& rhs)
{
    return lhs.name_ == rhs.name_ && lhs.value_ == rhs.value_;
}

} //namespace fcgi
