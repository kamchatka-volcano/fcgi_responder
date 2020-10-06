#pragma once
#include "message.h"
#include "namevalue.h"
#include <vector>

namespace fcgi{

class MsgParams : public Message{
public:
    MsgParams();
    void setParam(const std::string& name, const std::string& value);
    std::string paramValue(const std::string& name) const;
    std::vector<std::string> paramList() const;

private:
    void toStream(std::ostream& output) const override;
    void fromStream(std::istream& input) override;

private:
    std::vector<NameValue> paramList_;
};

}
