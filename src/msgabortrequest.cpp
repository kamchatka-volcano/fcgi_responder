#include "msgabortrequest.h"

using namespace fcgi;

MsgAbortRequest::MsgAbortRequest()
    : Message(RecordType::AbortRequest)
{
}

std::size_t MsgAbortRequest::size() const
{
    return 0;
}

void MsgAbortRequest::toStream(std::ostream&) const
{
}

void MsgAbortRequest::fromStream(std::istream&, std::size_t)
{
}

bool MsgAbortRequest::operator==(const MsgAbortRequest&) const
{
    return true;
}
