#pragma once
#include <ostream>

namespace fcgi{

class Encoder{
public:
    Encoder(std::ostream& stream);
    Encoder& operator<<(uint8_t val);
    Encoder& operator<<(uint16_t val);
    Encoder& operator<<(uint32_t val);
    Encoder& operator<<(const std::string& val);

    void addPadding(std::size_t numOfBytes);

private:
    std::ostream& output_;
};

}
