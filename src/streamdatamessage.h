#pragma once
#include "message.h"

namespace fcgi{

class StreamDataMessage : public Message<StreamDataMessage>{
    friend class Message<StreamDataMessage>;

public:
    template <typename TStr>
    StreamDataMessage(RecordType recordType, TStr&& data)
        : Message(recordType)
        , data_(std::forward<TStr>(data))
    {
    }

    std::size_t size() const;
    void setData(const std::string& data);
    const std::string& data() const;

private:
    void toStream(std::ostream& output) const;
    void fromStream(std::istream& input, std::size_t inputSize);

protected:
    std::string data_;
};

class MsgStdIn : public StreamDataMessage
{
public:
    MsgStdIn();
    template <typename TStr, typename TEnableIfString = std::enable_if_t<std::is_convertible_v<TStr, std::string >, void>>
    explicit MsgStdIn(TStr&& data)
        : StreamDataMessage(RecordType::StdIn, std::forward<TStr>(data))
    {}
    bool operator==(const MsgStdIn& other) const;
};

class MsgStdOut : public StreamDataMessage
{
public:
    MsgStdOut();
    template <typename TStr, typename TEnableIfString = std::enable_if_t<std::is_convertible_v<TStr, std::string >, void>>
    explicit MsgStdOut(TStr&& data)
        : StreamDataMessage(RecordType::StdOut, std::forward<TStr>(data))
    {}
    bool operator==(const MsgStdOut& other) const;
};

class MsgStdErr : public StreamDataMessage
{
public:
    MsgStdErr();
    template <typename TStr, typename TEnableIfString = std::enable_if_t<std::is_convertible_v<TStr, std::string >, void>>
    explicit MsgStdErr(TStr&& data)
        : StreamDataMessage(RecordType::StdErr, std::forward<TStr>(data))
    {}
    bool operator==(const MsgStdErr& other) const;
};

class MsgData : public StreamDataMessage
{
public:
    MsgData();
    template <typename TStr, typename TEnableIfString = std::enable_if_t<std::is_convertible_v<TStr, std::string >, void>>
    explicit MsgData(TStr&& data)
        : StreamDataMessage(RecordType::Data, std::forward<TStr>(data))
    {}
    bool operator==(const MsgData& other) const;
};

}
