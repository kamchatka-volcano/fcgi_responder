#pragma once
#include "message.h"
#include "namevalue.h"
#include <vector>

namespace fcgi{

class MsgParams : public Message{
public:
    MsgParams();
    std::size_t size() const override;
    const std::string& paramValue(const std::string& name) const;
    std::vector<std::string> paramList() const;
    bool operator==(const MsgParams& other) const;

    void setParam(const std::string& name, const std::string& value);

private:
    void toStream(std::ostream& output) const override;
    void fromStream(std::istream& input, std::size_t inputSize) override;

private:
    std::vector<NameValue> paramList_;
};

}
