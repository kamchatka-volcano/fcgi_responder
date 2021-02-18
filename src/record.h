#pragma once
#include "types.h"
#include "errors.h"
#include <cstdint>
#include <string>
#include <istream>
#include <ostream>
#include <sstream>
#include <memory>
#include <cassert>
#include "msgbeginrequest.h"
#include "msgendrequest.h"
#include "msggetvalues.h"
#include "msggetvaluesresult.h"
#include "msgparams.h"
#include "msgunknowntype.h"
#include "msgabortrequest.h"
#include "streamdatamessage.h"
#include <variant>

namespace fcgi{

class Record{
public:
    Record();
    Record(RecordType type, uint16_t requestId = 0);
    template <typename TMessage>
    Record(TMessage&& msg, uint16_t requestId = 0)
        : type_(msg.recordType())
        , requestId_(requestId)
        , message_(std::forward<TMessage>(msg))
    {
    }

    RecordType type() const;
    uint16_t requestId() const;
    std::size_t size() const;

    void toStream(std::ostream& output) const;
    std::size_t fromStream(std::istream& input, std::size_t inputSize);

    template <typename MsgT>
    const MsgT& getMessage() const;

    bool operator==(const Record& other) const;

private:
    void initMessage();
    std::size_t messageSize() const;
    void readMessage(std::istream &input, std::size_t inputSize);
    void writeMessage(std::ostream &output) const;


    void write(std::ostream& output) const;
    std::size_t read(std::istream& input, std::size_t inputSize);
    uint8_t calcPaddingLength() const;

private:
    RecordType type_;
    uint16_t requestId_;
    std::variant<MsgAbortRequest,
                 MsgBeginRequest,
                 MsgEndRequest,
                 MsgGetValues,
                 MsgGetValuesResult,
                 MsgParams,
                 MsgUnknownType,
                 MsgStdIn,
                 MsgStdOut,
                 MsgStdErr,
                 MsgData> message_;
};

template <typename MsgT>
const MsgT& Record::getMessage() const
{
    return std::get<MsgT>(message_);
}

}
