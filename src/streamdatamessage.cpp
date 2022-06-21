#include "streamdatamessage.h"

namespace fcgi{

StreamDataMessage::StreamDataMessage(RecordType recordType)
    : Message(recordType)
    , data_{std::string{}}
{
}

StreamDataMessage::StreamDataMessage(RecordType recordType, std::string_view data)
    : Message(recordType)
    , data_{data}
{
}

std::size_t StreamDataMessage::size() const
{
    return std::visit([](auto& data){return data.size();}, data_);
}

std::string_view StreamDataMessage::data() const
{
    return std::visit([](auto& data){return std::string_view{data};}, data_);
}

void StreamDataMessage::toStream(std::ostream& output) const
{
    output.write(data().data(), static_cast<int>(data().size()));
}

void StreamDataMessage::fromStream(std::istream& input, std::size_t inputSize)
{
    auto& data = std::get<std::string>(data_);
    data.resize(inputSize);
    input.read(&data[0], static_cast<std::streamsize>(inputSize));
}

MsgStdIn::MsgStdIn()
    : StreamDataMessage(RecordType::StdIn)
{}

MsgStdIn::MsgStdIn(std::string_view data)
    : StreamDataMessage(RecordType::StdIn, data)
{}

bool operator==(const MsgStdIn& lhs, const MsgStdIn& rhs)
{
    return lhs.data() == rhs.data();
}

MsgStdOut::MsgStdOut()
    : StreamDataMessage(RecordType::StdOut)
{}

MsgStdOut::MsgStdOut(std::string_view data)
    : StreamDataMessage(RecordType::StdOut, data)
{}

bool operator==(const MsgStdOut& lhs, const MsgStdOut& rhs)
{
    return lhs.data() == rhs.data();
}

MsgStdErr::MsgStdErr()
    : StreamDataMessage(RecordType::StdErr)
{}

MsgStdErr::MsgStdErr(std::string_view data)
    : StreamDataMessage(RecordType::StdErr, data)
{}

bool operator==(const MsgStdErr& lhs, const MsgStdErr& rhs)
{
    return lhs.data() == rhs.data();
}

MsgData::MsgData()
    : StreamDataMessage(RecordType::Data)
{}

MsgData::MsgData(std::string_view data)
    : StreamDataMessage(RecordType::Data, data)
{}

bool operator==(const MsgData& lhs, const MsgData& rhs)
{
    return lhs.data() == rhs.data();
}

}
