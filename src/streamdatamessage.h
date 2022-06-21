#pragma once
#include "message.h"
#include <variant>
#include <string_view>

namespace fcgi{

class StreamDataMessage : public Message<StreamDataMessage>{
    friend class Message<StreamDataMessage>;

public:
    explicit StreamDataMessage(RecordType recordType);
    StreamDataMessage(RecordType recordType, std::string_view data);

    std::size_t size() const;
    std::string_view data() const;

private:
    void toStream(std::ostream& output) const;
    void fromStream(std::istream& input, std::size_t inputSize);

private:
    std::variant<std::string, std::string_view> data_;
};

class MsgStdIn : public StreamDataMessage
{
public:
    MsgStdIn();
    explicit MsgStdIn(std::string_view data);
};
bool operator==(const MsgStdIn& lhs, const MsgStdIn& rhs);

class MsgStdOut : public StreamDataMessage
{
public:
    MsgStdOut();
    explicit MsgStdOut(std::string_view data);
};
bool operator==(const MsgStdOut& lhs, const MsgStdOut& rhs);

class MsgStdErr : public StreamDataMessage
{
public:
    MsgStdErr();
    explicit MsgStdErr(std::string_view data);
};
bool operator==(const MsgStdErr& lhs, const MsgStdErr& rhs);

class MsgData : public StreamDataMessage
{
public:
    MsgData();
    explicit MsgData(std::string_view data);
};
bool operator==(const MsgData& lhs, const MsgData& rhs);

}
