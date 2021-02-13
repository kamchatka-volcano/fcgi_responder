#pragma once
#include "message.h"
#include "namevalue.h"
#include <string>
#include <vector>

namespace fcgi{

class MsgGetValuesResult : public Message<MsgGetValuesResult>{
    friend class Message<MsgGetValuesResult>;

public:
    MsgGetValuesResult();
    const std::string& requestValue(ValueRequest request) const;
    std::vector<ValueRequest> requestList() const;
    void setRequestValue(ValueRequest request, const std::string& value);
    bool operator==(const MsgGetValuesResult& other) const;
    std::size_t size() const;

private:
    void toStream(std::ostream& output) const;
    void fromStream(std::istream& input, std::size_t inputSize);

private:
    std::vector<NameValue> requestValueList_;
};

}

