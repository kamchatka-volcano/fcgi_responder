#include "message.h"
#include "errors.h"

using namespace fcgi;

Message::Message(RecordType type)
    : recordType_(type)
{
}

Message::~Message()
{
}

void Message::read(std::istream &input)
{
    input.exceptions(std::istream::failbit | std::istream::badbit | std::istream::eofbit);
    try{
        fromStream(input);
    }
    catch(...){
        throw MessageReadError{};
    }
}

void Message::write(std::ostream &output) const
{
    output.exceptions(std::ostream::failbit | std::ostream::badbit | std::ostream::eofbit);
    try{
        toStream(output);
    }
    catch(...){
        throw MessageWriteError{};
    }
}

RecordType Message::recordType() const
{
    return recordType_;
}

