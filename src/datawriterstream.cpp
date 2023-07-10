#include "datawriterstream.h"

namespace fcgi {

DataWriterBuffer::DataWriterBuffer(std::size_t bufferMaxSize)
{
    buffer_.resize(bufferMaxSize);
    setp(buffer_.data(), buffer_.data() + buffer_.size());
}

const std::string& DataWriterBuffer::data() const
{
    return buffer_;
}

void DataWriterBuffer::reset(std::size_t size)
{
    buffer_.resize(size);
    setp(buffer_.data(), buffer_.data() + buffer_.size());
}

DataWriterStream::DataWriterStream(std::size_t bufferMaxSize)
    : DataWriterBuffer{bufferMaxSize}
    , std::ostream{this}
{
}

const std::string& DataWriterStream::buffer() const
{
    return DataWriterBuffer::data();
}

void DataWriterStream::resetBuffer(std::size_t size)
{
    DataWriterBuffer::reset(size);
}

} //namespace fcgi
