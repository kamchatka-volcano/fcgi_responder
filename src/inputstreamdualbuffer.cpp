#include "inputstreamdualbuffer.h"

using namespace fcgi;

InputStreamDualBuffer::InputStreamDualBuffer(
        const char* firstBuf, std::size_t firstBufSize,
        const char* secondBuf, std::size_t secondBufSize)
    : firstBuf_(const_cast<char*>(firstBuf))
    , secondBuf_(const_cast<char*>(secondBuf))
    , firstBufSize_(firstBufSize)
    , secondBufSize_(secondBufSize)
{
    if (!firstBufSize_)
        firstBufIsUsed_ = true;
    if (!secondBufSize_)
        secondBufIsUsed_ = true;
}

int InputStreamDualBuffer::underflow()
{
    if (gptr() == egptr()) {
        if (!firstBufIsUsed_){
            setg(firstBuf_, firstBuf_, firstBuf_ + firstBufSize_);
            firstBufIsUsed_ = true;
        }
        else if (!secondBufIsUsed_){
            setg(secondBuf_, secondBuf_, secondBuf_ + secondBufSize_);
            secondBufIsUsed_ = true;
        }
    }
    return gptr() == egptr()
         ? std::char_traits<char>::eof()
         : std::char_traits<char>::to_int_type(*gptr());
}
