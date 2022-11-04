#include "datareaderstream.h"

namespace fcgi{

MultiPartDataReaderBuffer::MultiPartDataReaderBuffer(std::vector<std::string_view> buffers)
    : buffers_{std::move(buffers)}
{}

int MultiPartDataReaderBuffer::underflow()
{
    if (gptr() == egptr()) {
        while (bufferIndex_ < buffers_.size()) {
            auto buffer = buffers_[bufferIndex_++];
            if (!buffer.empty()) {
                auto bufferPtr = const_cast<char*>(buffer.data());
                setg(bufferPtr, bufferPtr, bufferPtr + buffer.size());
                break;
            }
        }
    }
    return gptr() == egptr()
           ? std::char_traits<char>::eof()
           : std::char_traits<char>::to_int_type(*gptr());
}


}
