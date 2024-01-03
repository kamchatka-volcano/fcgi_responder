#pragma once
#include <cstdint>
#include <ostream>

namespace fcgi {

class Encoder {
public:
    explicit Encoder(std::ostream& stream);
    Encoder& operator<<(std::uint8_t val);
    Encoder& operator<<(std::uint16_t val);
    Encoder& operator<<(std::uint32_t val);
    Encoder& operator<<(const std::string& val);

    void addPadding(std::size_t numOfBytes);

private:
    std::ostream& output_;
};

} //namespace fcgi
