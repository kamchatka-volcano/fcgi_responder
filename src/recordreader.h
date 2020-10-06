#pragma once
#include <string>
#include <functional>

namespace fcgi{

class Record;

class RecordReader{
public:
    RecordReader(std::function<void(const Record&)> recordReadedHandler);
    void addData(const std::string& data);
    void clear();

private:
    std::string buffer_;
    std::function<void(const Record&)> recordReadedHandler_;
};

}

