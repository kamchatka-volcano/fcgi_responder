#pragma once
#include "message.h"
#include <variant>
#include <string_view>

namespace fcgi{

template <RecordType recordType>
class StreamDataMessage : public Message<StreamDataMessage<recordType>>{
    friend class Message<StreamDataMessage<recordType>>;

public:
    StreamDataMessage()
        : Message<StreamDataMessage>(recordType)
        , data_{std::string{}}
    {}

    explicit StreamDataMessage(std::string_view data)
        : Message<StreamDataMessage>(recordType)
        , data_{data}
    {}

    std::size_t size() const
    {
        return std::visit([](auto& data){return data.size();}, data_);
    }
    std::string_view data() const
    {
        return std::visit([](auto& data){return std::string_view{data};}, data_);
    }

private:
    void toStream(std::ostream& output) const
    {
        output.write(data().data(), static_cast<int>(data().size()));
    }
    void fromStream(std::istream& input, std::size_t inputSize)
    {
        auto& data = std::get<std::string>(data_);
        data.resize(inputSize);
        input.read(&data[0], static_cast<std::streamsize>(inputSize));
    }

private:
    std::variant<std::string, std::string_view> data_;
};

template<RecordType recordType>
bool operator==(const StreamDataMessage<recordType>& lhs, const StreamDataMessage<recordType>& rhs)
{
    return lhs.data() == rhs.data();
}

using MsgStdIn  = StreamDataMessage<RecordType::StdIn>;
using MsgStdOut = StreamDataMessage<RecordType::StdOut>;
using MsgStdErr = StreamDataMessage<RecordType::StdErr>;
using MsgData   = StreamDataMessage<RecordType::Data>;

}
