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
    Record(const Message& msg, uint16_t requestId = 0);
    RecordType type() const;
    uint16_t requestId() const;
    const std::string& messageData() const;

    void toStream(std::ostream& output) const;
    int fromStream(std::istream& input);

    template <typename MsgT>
    MsgT getMessage() const;

private:
    void write(std::ostream& output) const;
    int read(std::istream& input);

private:
    RecordType type_;
    uint16_t requestId_;
    std::string messageData_;
};

template <typename MsgT>
MsgT Record::getMessage() const
{
    auto msgStream = std::istringstream{messageData_};
    auto msg = MsgT{};
    assert(type_ == msg.recordType());

    msg.read(msgStream);
    return msg;
}

}
