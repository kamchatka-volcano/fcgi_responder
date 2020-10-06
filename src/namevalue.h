#pragma once
#include <string>
#include <istream>
#include <ostream>

namespace fcgi{

class NameValue{
public:
    NameValue();
    NameValue(const std::string& name, const std::string& value);

    const std::string& name() const;
    const std::string& value() const;
    void setName(const std::string& name);
    void setValue(const std::string& value);

    void toStream(std::ostream& output) const;
    void fromStream(std::istream& input);

private:
    std::string name_;
    std::string value_;
};

}
