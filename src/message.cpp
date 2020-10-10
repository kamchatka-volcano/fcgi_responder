#include "message.h"
#include "errors.h"
#include "msgabortrequest.h"
#include "msgbeginrequest.h"
#include "msgendrequest.h"
#include "msggetvalues.h"
#include "msggetvaluesresult.h"
#include "msgparams.h"
#include "msgunknowntype.h"
#include "streamdatamessage.h"

using namespace fcgi;

Message::Message(RecordType type)
    : recordType_(type)
{
}

Message::~Message()
{
}

void Message::read(std::istream &input, std::size_t inputSize)
{
    if (inputSize == 0)
        return;
    if (inputSize < size())
        throw MessageReadError{};
    input.exceptions(std::istream::failbit | std::istream::badbit | std::ostream::eofbit);
    try{
        fromStream(input, inputSize);
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

std::unique_ptr<Message> Message::createMessage(RecordType type)
{
    switch(type){
    case RecordType::BeginRequest: return std::make_unique<MsgBeginRequest>();
    case RecordType::AbortRequest: return std::make_unique<MsgAbortRequest>();
    case RecordType::EndRequest: return std::make_unique<MsgEndRequest>();
    case RecordType::Params: return std::make_unique<MsgParams>();
    case RecordType::StdIn: return std::make_unique<MsgStdIn>();
    case RecordType::StdOut: return std::make_unique<MsgStdOut>();
    case RecordType::StdErr: return std::make_unique<MsgStdErr>();
    case RecordType::Data: return std::make_unique<MsgData>();
    case RecordType::GetValues: return std::make_unique<MsgGetValues>();
    case RecordType::GetValuesResult: return std::make_unique<MsgGetValuesResult>();
    case RecordType::UnknownType: return std::make_unique<MsgUnknownType>();
    default: return nullptr;
    }
}

