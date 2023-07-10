#pragma once
#include <istream>

namespace fcgi {

class Decoder {
public:
    explicit Decoder(std::istream& input);
    Decoder& operator>>(uint8_t& val);
    Decoder& operator>>(uint16_t& val);
    Decoder& operator>>(uint32_t& val);
    Decoder& operator>>(std::string& val);

    void skip(std::size_t numOfBytes);

private:
    std::istream& input_;
};

} //namespace fcgi
