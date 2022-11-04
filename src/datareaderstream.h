#pragma once
#include <istream>
#include <streambuf>
#include <string_view>
#include <vector>

namespace fcgi{

class MultiPartDataReaderBuffer : public std::streambuf {
public:
    explicit MultiPartDataReaderBuffer(std::vector<std::string_view> buffers);
    int underflow() override;

private:
    std::vector<std::string_view> buffers_;
    std::size_t bufferIndex_ = 0;
};

class DataReaderStream : private MultiPartDataReaderBuffer, public std::istream {
public:
    template<typename... T>
    explicit DataReaderStream(T&&... buffers)
        : MultiPartDataReaderBuffer{{std::forward<T>(buffers)...}}
        , std::istream{this}
    {
        static_assert((... && std::is_convertible_v<T, std::string_view>));
    }
};

}