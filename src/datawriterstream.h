#pragma once
#include <ostream>
#include <streambuf>

namespace fcgi {

class DataWriterBuffer : public std::streambuf {
public:
    explicit DataWriterBuffer(std::size_t bufferMaxSize);
    const std::string& data() const;
    void reset(std::size_t size);

private:
    std::string buffer_;
};

class DataWriterStream : private DataWriterBuffer,
                         public std::ostream {
public:
    explicit DataWriterStream(std::size_t bufferMaxSize);
    const std::string& buffer() const;
    void resetBuffer(std::size_t size);
};

} //namespace fcgi