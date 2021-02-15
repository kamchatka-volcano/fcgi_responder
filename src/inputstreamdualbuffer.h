#pragma once
#include <streambuf>

namespace fcgi{

class InputStreamDualBuffer : public std::streambuf {
public:
    InputStreamDualBuffer(const char* firstBuf = nullptr, std::size_t firstBufSize = 0,
                          const char* secondBuf = nullptr, std::size_t secondBufSize = 0);
    int underflow() override;

private:
    bool firstBufIsUsed_ = false;
    bool secondBufIsUsed_ = false;
    char* firstBuf_;
    char* secondBuf_;
    std::size_t firstBufSize_;
    std::size_t secondBufSize_;
};

}
