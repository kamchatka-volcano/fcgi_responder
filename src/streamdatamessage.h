#pragma once
#include "message.h"

namespace fcgi{

class StreamDataMessage : public Message{
public:
    StreamDataMessage(RecordType recordType, const std::string& data = {});
    void setData(const std::string& data);
    const std::string& data() const;

private:
    void toStream(std::ostream& output) const override;
    void fromStream(std::istream& input) override;

private:
    std::string data_;
};

class MsgStdIn : public StreamDataMessage
{
public:
    MsgStdIn(const std::string& data = {});
};

class MsgStdOut : public StreamDataMessage
{
public:
    MsgStdOut(const std::string& data = {});
};

class MsgStdErr : public StreamDataMessage
{
public:
    MsgStdErr(const std::string& data = {});
};

class MsgData : public StreamDataMessage
{
public:
    MsgData(const std::string& data = {});
};

}
