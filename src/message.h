#pragma once
#include "types.h"
#include <string>
#include <istream>
#include <ostream>
#include <memory>

namespace fcgi{

class Message{
public:
    Message(RecordType type);
    virtual ~Message();
    virtual std::size_t size() const = 0;
    RecordType recordType() const;

    void write(std::ostream& output) const;
    void read(std::istream& input, std::size_t inputSize);

static std::unique_ptr<Message> createMessage(RecordType type);

private:
    virtual void toStream(std::ostream& output) const = 0;
    virtual void fromStream(std::istream& input, std::size_t inputSize) = 0;

private:
    RecordType recordType_;
};

}


