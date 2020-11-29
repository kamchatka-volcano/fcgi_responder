#include "streamdatamessage.h"

namespace fcgi{

StreamDataMessage::StreamDataMessage(RecordType recordType, const std::string& data )
    : Message(recordType)
    , data_(data)
{
}

std::size_t StreamDataMessage::size() const
{
    return data_.size();
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

void StreamDataMessage::fromStream(std::istream& input, std::size_t inputSize)
{
    data_.resize(inputSize);
    input.read(&data_[0], static_cast<std::streamsize>(inputSize));
}

MsgStdIn::MsgStdIn(const std::string& data)
    : StreamDataMessage(RecordType::StdIn, data)
{
}

bool MsgStdIn::operator==(const MsgStdIn& other) const
{
    return data_ == other.data_;
}

MsgStdOut::MsgStdOut(const std::string& data)
    : StreamDataMessage(RecordType::StdOut, data)
{
}

bool MsgStdOut::operator==(const MsgStdOut& other) const
{
    return data_ == other.data_;
}

MsgStdErr::MsgStdErr(const std::string& data)
    : StreamDataMessage(RecordType::StdErr, data)
{
}

bool MsgStdErr::operator==(const MsgStdErr& other) const
{
    return data_ == other.data_;
}

MsgData::MsgData(const std::string& data)
    : StreamDataMessage(RecordType::Data, data)
{
}

bool MsgData::operator==(const MsgData& other) const
{
    return data_ == other.data_;
}

}
