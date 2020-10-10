#pragma once
#include "types.h"
#include "errors.h"
#include <cstdint>
#include <string>
#include <istream>
#include <ostream>
#include <sstream>
#include <memory>
#include <cassert>

namespace fcgi{

class Message;

class Record{
public:
    Record();
    Record(RecordType type, uint16_t requestId = 0);
    Record(std::unique_ptr<Message> msg, uint16_t requestId = 0);
    Record(Record&& record);
    ~Record();
    RecordType type() const;
    uint16_t requestId() const;
    std::size_t size() const;

    void toStream(std::ostream& output) const;
    std::size_t fromStream(std::istream& input, std::size_t inputSize);

    template <typename MsgT>
    MsgT& getMessage() const;

    bool operator==(const Record& other) const;

private:
    void write(std::ostream& output) const;
    std::size_t read(std::istream& input, std::size_t inputSize);
    RecordType messageType(const Message& msg) const;
    uint8_t calcPaddingLength() const;

private:
    RecordType type_;
    uint16_t requestId_;
    std::unique_ptr<Message> message_;
};

template <typename MsgT>
MsgT& Record::getMessage() const
{
    assert(message_);
    assert(type_ == messageType(*message_));
    return *static_cast<MsgT*>(message_.get());
}

}
