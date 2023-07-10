#include "msgabortrequest.h"

namespace fcgi {

std::size_t MsgAbortRequest::size()
{
    return 0;
}

void MsgAbortRequest::toStream(std::ostream&) const {}

void MsgAbortRequest::fromStream(std::istream&, std::size_t) {}

bool operator==(const MsgAbortRequest&, const MsgAbortRequest&)
{
    return true;
}

} //namespace fcgi
