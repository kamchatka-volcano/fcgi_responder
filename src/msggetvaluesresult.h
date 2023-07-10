#pragma once
#include "namevalue.h"
#include "types.h"
#include <istream>
#include <ostream>
#include <string>
#include <vector>

namespace fcgi {

class MsgGetValuesResult {
public:
    static const RecordType recordType = RecordType::GetValuesResult;

public:
    const std::string& requestValue(ValueRequest request) const;
    std::vector<ValueRequest> requestList() const;
    void setRequestValue(ValueRequest request, const std::string& value);
    std::size_t size() const;

    void toStream(std::ostream& output) const;
    void fromStream(std::istream& input, std::size_t inputSize);

private:
    friend bool operator==(const MsgGetValuesResult& lhs, const MsgGetValuesResult& rhs);

private:
    std::vector<NameValue> requestValueList_;
};

bool operator==(const MsgGetValuesResult& lhs, const MsgGetValuesResult& rhs);

} //namespace fcgi
