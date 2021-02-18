#include "streamdatamessage.h"

namespace fcgi{

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

MsgStdIn::MsgStdIn()
    : StreamDataMessage(RecordType::StdIn, std::string{})
{}

bool MsgStdIn::operator==(const MsgStdIn& other) const
{
    return data_ == other.data_;
}

MsgStdOut::MsgStdOut()
    : StreamDataMessage(RecordType::StdOut, std::string{})
{}

bool MsgStdOut::operator==(const MsgStdOut& other) const
{
    return data_ == other.data_;
}

MsgStdErr::MsgStdErr()
    : StreamDataMessage(RecordType::StdErr, std::string{})
{}

bool MsgStdErr::operator==(const MsgStdErr& other) const
{
    return data_ == other.data_;
}

MsgData::MsgData()
    : StreamDataMessage(RecordType::Data, std::string{})
{}

bool MsgData::operator==(const MsgData& other) const
{
    return data_ == other.data_;
}

}
