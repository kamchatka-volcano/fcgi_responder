#pragma once
#include "types.h"
#include <string>
#include <istream>
#include <ostream>

namespace fcgi{

class Message{
public:
    Message(RecordType type);
    virtual ~Message();
    RecordType recordType() const;

    void write(std::ostream& output) const;
    void read(std::istream& input);

private:
    virtual void toStream(std::ostream& output) const = 0;
    virtual void fromStream(std::istream& input) = 0;

private:
    RecordType recordType_;
};

}


