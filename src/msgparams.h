#pragma once
#include "namevalue.h"
#include "types.h"
#include <istream>
#include <ostream>
#include <vector>

namespace fcgi {

class MsgParams {
public:
    static const RecordType recordType = RecordType::Params;

public:
    const std::string& paramValue(const std::string& name) const;
    std::vector<std::string> paramList() const;
    void setParam(const std::string& name, const std::string& value);
    std::size_t size() const;

    void toStream(std::ostream& output) const;
    void fromStream(std::istream& input, std::size_t inputSize);

private:
    friend bool operator==(const MsgParams& lhs, const MsgParams& rhs);

private:
    std::vector<NameValue> paramList_;
};

bool operator==(const MsgParams& lhs, const MsgParams& rhs);

} //namespace fcgi
