#pragma once
#include <string>
#include <istream>
#include <ostream>

namespace fcgi{

class NameValue{
public:
    NameValue();
    NameValue(const std::string& name, const std::string& value);
    NameValue(std::string&& name, std::string&& value);

    std::size_t size() const;
    const std::string& name() const;
    const std::string& value() const;

    void setName(const std::string& name);
    void setValue(const std::string& value);

    void toStream(std::ostream& output) const;
    void fromStream(std::istream& input);

    bool operator==(const NameValue& other) const;

private:
    std::string name_;
    std::string value_;
};

}
