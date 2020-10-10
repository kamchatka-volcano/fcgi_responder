#pragma once
#include "message.h"

namespace fcgi{

class StreamDataMessage : public Message{
public:
    StreamDataMessage(RecordType recordType, const std::string& data = {});
    std::size_t size() const override;
    void setData(const std::string& data);
    const std::string& data() const;

private:
    void toStream(std::ostream& output) const override;
    void fromStream(std::istream& input, std::size_t inputSize) override;

protected:
    std::string data_;
};

class MsgStdIn : public StreamDataMessage
{
public:
    MsgStdIn(const std::string& data = {});
    bool operator==(const MsgStdIn& other) const;
};

class MsgStdOut : public StreamDataMessage
{
public:
    MsgStdOut(const std::string& data = {});
    bool operator==(const MsgStdOut& other) const;
};

class MsgStdErr : public StreamDataMessage
{
public:
    MsgStdErr(const std::string& data = {});
    bool operator==(const MsgStdErr& other) const;
};

class MsgData : public StreamDataMessage
{
public:
    MsgData(const std::string& data = {});
    bool operator==(const MsgData& other) const;
};

}
