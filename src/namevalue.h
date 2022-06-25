#pragma once
#include <string>
#include <istream>
#include <ostream>

namespace fcgi{

class NameValue{
public:
    explicit NameValue(std::size_t maxSize);

    template<typename TStrName, typename TStrValue>
    NameValue(TStrName&& name, TStrValue&& value)
        : name_(std::forward<TStrName>(name))
        , value_(std::forward<TStrValue>(value))
    {}

    std::size_t size() const;
    const std::string& name() const;
    const std::string& value() const;

    void setName(const std::string& name);
    void setValue(const std::string& value);

    void toStream(std::ostream& output) const;
    void fromStream(std::istream& input);

private:
    friend bool operator==(const NameValue& lhs, const NameValue& rhs);

private:
    std::string name_;
    std::string value_;
    std::size_t maxSize_ = 0;
};

bool operator==(const NameValue& lhs, const NameValue& rhs);

}
