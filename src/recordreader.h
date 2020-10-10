#pragma once
#include <string>
#include <functional>

namespace fcgi{

class Record;

class RecordReader{
public:
    RecordReader(std::function<void(Record&)> recordReadedHandler);
    void addData(const std::string& data);
    void clear();

private:
    std::string buffer_;
    std::size_t readedBytes_ = 0;
    std::function<void(Record&)> recordReadedHandler_;
};

}

