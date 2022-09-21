#pragma once
#include <istream>
#include <streambuf>
#include <string_view>
#include <vector>

namespace sfun::stream_utils{

class MultiPartBuffer : public std::streambuf {
public:
    MultiPartBuffer(std::vector<std::string_view> buffers)
        : buffers_{std::move(buffers)}
    {}

    int underflow() override
    {
        if (gptr() == egptr() && bufferIndex_ < buffers_.size()) {
            auto buffer = buffers_[bufferIndex_++];
            auto bufferPtr = const_cast<char*>(buffer.data());
            setg(bufferPtr, bufferPtr, bufferPtr + buffer.size());
        }
        return gptr() == egptr()
             ? std::char_traits<char>::eof()
             : std::char_traits<char>::to_int_type(*gptr());
    }

private:
    std::vector<std::string_view> buffers_;
    std::size_t bufferIndex_ = 0;
};

class MultiPartStream : public std::istream {
public:
    MultiPartStream(std::vector<std::string_view> buffers)
        : buffer_(std::move(buffers))
    {
        rdbuf(&buffer_);
    }

private:
    MultiPartBuffer buffer_;
};

}