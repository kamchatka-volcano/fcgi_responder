#include "namevalue.h"
#include "decoder.h"
#include "encoder.h"

namespace fcgi{

NameValue::NameValue()
{
}

NameValue::NameValue(const std::string& name, const std::string& value)
    : name_(name)
    , value_(value)
{
}

NameValue::NameValue(std::string&& name, std::string&& value)
    : name_(std::move(name))
    , value_(std::move(value))
{
}

std::size_t NameValue::size() const
{
    auto result = 0u;
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

uint32_t readLengthFromStream(std::istream& input)
{
    auto ch = char{};
    input.get(ch);

    auto length = 0;
    if ((ch >> 7) == 0)
        length = ch;
    else{
        auto lengthB3 = static_cast<unsigned char>(ch);
        input.get(ch);
        auto lengthB2 = static_cast<unsigned char>(ch);
        input.get(ch);
        auto lengthB1 = static_cast<unsigned char>(ch);
        input.get(ch);
        auto lengthB0 = static_cast<unsigned char>(ch);

        length = ((lengthB3 & 0x7f) << 24) + (lengthB2 << 16) + (lengthB1 << 8) + lengthB0;
    }
    return static_cast<uint32_t>(length);
}

void writeLengthToStream(uint32_t length, std::ostream& output)
{
    auto encoder = Encoder(output);
    if (length <= 127){        
        encoder << static_cast<uint8_t>(length);
    }
    else{
        length |= 0x80000000;
        encoder << length;
    }
}

}

void NameValue::toStream(std::ostream &output) const
{
    writeLengthToStream(static_cast<uint32_t>(name_.size()), output);
    writeLengthToStream(static_cast<uint32_t>(value_.size()), output);
    auto encoder = Encoder(output);
    encoder << name_
            << value_;
}

void NameValue::fromStream(std::istream& input)
{
    auto nameLength = readLengthFromStream(input);
    auto valueLength = readLengthFromStream(input);
    name_.resize(nameLength);
    value_.resize(valueLength);
    auto decoder = Decoder(input);
    decoder >> name_
            >> value_;
}

bool NameValue::operator==(const NameValue& other) const
{
    return name_ == other.name_ &&
           value_ == other.value_;
}

}
