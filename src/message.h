#pragma once
#include "types.h"
#include <string>
#include <istream>
#include <ostream>
#include <memory>

namespace fcgi{

template <typename TMsg>
class Message{
public:
    explicit Message(RecordType type)
        : recordType_(type)
    {}

    RecordType recordType() const
    {
        return recordType_;
    }

    void write(std::ostream &output) const
    {
        output.exceptions(std::ostream::failbit | std::ostream::badbit | std::ostream::eofbit);
        static_cast<const TMsg*>(this)->toStream(output);
    }

    void read(std::istream &input, std::size_t inputSize)
    {
        if (inputSize == 0)
            return;
        input.exceptions(std::istream::failbit | std::istream::badbit | std::ostream::eofbit);
        static_cast<TMsg*>(this)->fromStream(input, inputSize);
    }

private:
    RecordType recordType_;
};

}


