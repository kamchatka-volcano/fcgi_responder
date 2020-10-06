#pragma once
#include "message.h"
#include <map>
#include <string>
#include <vector>

namespace fcgi{

class MsgGetValuesResult : public Message{
public:
    MsgGetValuesResult();
    void setRequestValue(ValueRequest request, const std::string value);
    std::string requestValue(ValueRequest request) const;
    std::vector<ValueRequest> requestList() const;

private:
    void toStream(std::ostream& output) const override;
    void fromStream(std::istream& input) override;

private:
    std::map<ValueRequest, std::string> requestValueMap_;
};

}

