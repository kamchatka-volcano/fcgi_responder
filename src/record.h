#pragma once
#include "errors.h"
#include "msgabortrequest.h"
#include "msgbeginrequest.h"
#include "msgendrequest.h"
#include "msggetvalues.h"
#include "msggetvaluesresult.h"
#include "msgparams.h"
#include "msgunknowntype.h"
#include "streamdatamessage.h"
#include "types.h"
#include <cstdint>
#include <istream>
#include <memory>
#include <ostream>
#include <variant>

namespace fcgi {

class Record {
public:
    Record();
    explicit Record(RecordType type, uint16_t requestId = 0);
    template<typename TMessage>
    Record(TMessage&& msg, uint16_t requestId)
        : type_(std::remove_reference_t<TMessage>::recordType)
        , requestId_(requestId)
        , message_(std::forward<TMessage>(msg))
    {
    }

    RecordType type() const;
    uint16_t requestId() const;
    std::size_t size() const;

    void toStream(std::ostream& output) const;
    std::size_t fromStream(std::istream& input, std::size_t inputSize);

    template<typename MsgT>
    const MsgT& getMessage() const;

    friend bool operator==(const Record& lhs, const Record& rhs);

private:
    void initMessage();
    std::size_t messageSize() const;
    void readMessage(std::istream& input, std::size_t inputSize);
    void writeMessage(std::ostream& output) const;

    void write(std::ostream& output) const;
    std::size_t read(std::istream& input, std::size_t inputSize);
    uint8_t calcPaddingLength() const;

private:
    RecordType type_ = RecordType::UnknownType;
    uint16_t requestId_ = 0;
    std::variant<
            MsgAbortRequest,
            MsgBeginRequest,
            MsgEndRequest,
            MsgGetValues,
            MsgGetValuesResult,
            MsgParams,
            MsgUnknownType,
            MsgStdIn,
            MsgStdOut,
            MsgStdErr,
            MsgData>
            message_;
};

template<typename MsgT>
const MsgT& Record::getMessage() const
{
    return std::get<MsgT>(message_);
}
bool operator==(const Record& lhs, const Record& rhs);

} //namespace fcgi
