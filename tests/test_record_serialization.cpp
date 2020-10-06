#include <msgbeginrequest.h>
#include <msgendrequest.h>
#include <msggetvalues.h>
#include <msggetvaluesresult.h>
#include <msgparams.h>
#include <msgunknowntype.h>
#include <streamdatamessage.h>
#include <record.h>
#include <gtest/gtest.h>
#include <sstream>

namespace{

bool operator==(const fcgi::Record& lhs, const fcgi::Record& rhs)
{
    return lhs.type() == rhs.type() &&
           lhs.requestId() == rhs.requestId() &&
           lhs.messageData() == rhs.messageData();
}

void convertRecordTest(const fcgi::Record& record, fcgi::Record& resultRecord)
{
    auto output = std::ostringstream{};
    record.toStream(output);
    auto recordData = output.str();

    auto input = std::istringstream{recordData};
    auto recordSize = resultRecord.fromStream(input);
    ASSERT_TRUE(recordSize != 0);
    ASSERT_TRUE(record == resultRecord);
}

}


TEST(RecordSerialization, MsgBeginRequest)
{
    auto msg = fcgi::MsgBeginRequest{fcgi::Role::Responder,
                                     fcgi::ResultConnectionState::KeepOpen};

    auto record = fcgi::Record{msg, 1};
    auto resultRecord = fcgi::Record{};
    convertRecordTest(record, resultRecord);

    auto readedMsg = resultRecord.getMessage<fcgi::MsgBeginRequest>();
    ASSERT_EQ(msg.role(), readedMsg.role());
    ASSERT_EQ(msg.resultConnectionState(), readedMsg.resultConnectionState());
}

TEST(RecordSerialization, MsgEndRequest)
{
    auto msg = fcgi::MsgEndRequest{77, fcgi::ProtocolStatus::Overloaded};
    auto record = fcgi::Record{msg, 1};
    auto resultRecord = fcgi::Record{};
    convertRecordTest(record, resultRecord);

    auto readedMsg = resultRecord.getMessage<fcgi::MsgEndRequest>();
    ASSERT_EQ(msg.appStatus(), readedMsg.appStatus());
    ASSERT_EQ(msg.protocolStatus(), readedMsg.protocolStatus());

}

TEST(RecordSerialization, MsgGetValues)
{
    auto msg = fcgi::MsgGetValues{};
    auto record = fcgi::Record{msg, 1};
    auto resultRecord = fcgi::Record{};
    convertRecordTest(record, resultRecord);

    auto readedMsg = resultRecord.getMessage<fcgi::MsgGetValues>();
    ASSERT_EQ(msg.requestList(), readedMsg.requestList());

    msg = fcgi::MsgGetValues{};
    msg.requestValue(fcgi::ValueRequest::MaxReqs);
    msg.requestValue(fcgi::ValueRequest::MpxsConns);
    record = fcgi::Record{msg, 1};
    resultRecord = fcgi::Record{};
    convertRecordTest(record, resultRecord);

    readedMsg = resultRecord.getMessage<fcgi::MsgGetValues>();
    ASSERT_EQ(msg.requestList(), readedMsg.requestList());
}

TEST(RecordSerialization, MsgGetValuesResult)
{
    auto msg = fcgi::MsgGetValuesResult{};
    auto record = fcgi::Record{msg, 1};
    auto resultRecord = fcgi::Record{};
    convertRecordTest(record, resultRecord);

    auto readedMsg = resultRecord.getMessage<fcgi::MsgGetValuesResult>();
    ASSERT_EQ(msg.requestList(), readedMsg.requestList());

    msg = fcgi::MsgGetValuesResult{};
    msg.setRequestValue(fcgi::ValueRequest::MaxReqs, "10");
    msg.setRequestValue(fcgi::ValueRequest::MpxsConns, "2");
    record = fcgi::Record{msg, 1};
    resultRecord = fcgi::Record{};
    convertRecordTest(record, resultRecord);

    readedMsg = resultRecord.getMessage<fcgi::MsgGetValuesResult>();
    ASSERT_EQ(msg.requestList(), readedMsg.requestList());
    for(auto request : msg.requestList())
        ASSERT_EQ(msg.requestValue(request), readedMsg.requestValue(request));
}

TEST(RecordSerialization, MsgParams)
{
    auto msg = fcgi::MsgParams{};
    auto record = fcgi::Record{msg, 1};
    auto resultRecord = fcgi::Record{};
    convertRecordTest(record, resultRecord);

    auto readedMsg = resultRecord.getMessage<fcgi::MsgParams>();
    ASSERT_EQ(msg.paramList(), readedMsg.paramList());

    msg = fcgi::MsgParams{};
    msg.setParam("Hello", "World");
    record = fcgi::Record{msg, 1};
    resultRecord = fcgi::Record{};
    convertRecordTest(record, resultRecord);

    readedMsg = resultRecord.getMessage<fcgi::MsgParams>();
    ASSERT_EQ(msg.paramList(), readedMsg.paramList());
    for(const auto& param : msg.paramList())
        ASSERT_EQ(msg.paramValue(param), readedMsg.paramValue(param));
}

TEST(RecordSerialization, MsgUnkownType)
{
    auto msg = fcgi::MsgUnknownType{77};
    auto record = fcgi::Record{msg, 1};
    auto resultRecord = fcgi::Record{};
    convertRecordTest(record, resultRecord);

    auto readedMsg = resultRecord.getMessage<fcgi::MsgUnknownType>();
    ASSERT_EQ(msg.unknownTypeValue(), readedMsg.unknownTypeValue());
}

TEST(RecordSerialization, StreamDataMessage)
{
    auto msg = fcgi::StreamDataMessage{fcgi::RecordType::StdIn, "Hello world"};
    auto record = fcgi::Record{msg, 1};
    auto resultRecord = fcgi::Record{};
    convertRecordTest(record, resultRecord);

    auto readedMsg = resultRecord.getMessage<fcgi::MsgStdIn>();
    ASSERT_EQ(msg.data(), readedMsg.data());
}
