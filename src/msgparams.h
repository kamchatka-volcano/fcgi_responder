#pragma once
#include "message.h"
#include "namevalue.h"
#include <vector>

namespace fcgi{

class MsgParams : public Message<MsgParams>{
    friend class Message<MsgParams>;

public:
    MsgParams();
    const std::string& paramValue(const std::string& name) const;
    std::vector<std::string> paramList() const;
    void setParam(const std::string& name, const std::string& value);
    bool operator==(const MsgParams& other) const;
    std::size_t size() const;

private:
    void toStream(std::ostream& output) const;
    void fromStream(std::istream& input, std::size_t inputSize);

private:
    std::vector<NameValue> paramList_;
};

}
