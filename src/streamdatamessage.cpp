#include "streamdatamessage.h"

using namespace fcgi;

StreamDataMessage::StreamDataMessage(RecordType recordType, const std::string& data )
    : Message(recordType)
    , data_(data)
{
}

void StreamDataMessage::setData(const std::string& data)
{
    data_ = data;
}

const std::string& StreamDataMessage::data() const
{
    return data_;
}

void StreamDataMessage::toStream(std::ostream& output) const
{
    output.write(&data_[0], static_cast<int>(data_.size()));
}

void StreamDataMessage::fromStream(std::istream& input)
{
    input.seekg (0, input.end);
    auto length = input.tellg();
    input.seekg (0, input.beg);

    data_.resize(static_cast<std::size_t>(length));
    input.read(&data_[0], length);
}

MsgStdIn::MsgStdIn(const std::string& data)
    : StreamDataMessage(RecordType::StdIn, data)
{
}

MsgStdOut::MsgStdOut(const std::string& data)
    : StreamDataMessage(RecordType::StdOut, data)
{
}

MsgStdErr::MsgStdErr(const std::string& data)
    : StreamDataMessage(RecordType::StdErr, data)
{
}

MsgData::MsgData(const std::string& data)
    : StreamDataMessage(RecordType::Data, data)
{
}
