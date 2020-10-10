#pragma once
#include "message.h"
#include <map>
#include <string>
#include <vector>

namespace fcgi{

class MsgGetValuesResult : public Message{
public:
    MsgGetValuesResult();
    std::size_t size() const override;
    void setRequestValue(ValueRequest request, const std::string value);
    std::string requestValue(ValueRequest request) const;
    std::vector<ValueRequest> requestList() const;
    bool operator==(const MsgGetValuesResult& other) const;

private:
    void toStream(std::ostream& output) const override;
    void fromStream(std::istream& input, std::size_t inputSize) override;

private:
    std::map<ValueRequest, std::string> requestValueMap_;
};

}

