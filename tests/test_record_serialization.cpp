#include <msgbeginrequest.h>
#include <msgendrequest.h>
#include <msggetvalues.h>
#include <msggetvaluesresult.h>
#include <msgparams.h>
#include <msgunknowntype.h>
#include <msgabortrequest.h>
#include <streamdatamessage.h>
#include <record.h>
#include <gtest/gtest.h>
#include <sstream>

namespace{

void convertRecordTest(const fcgi::Record& record, fcgi::Record& resultRecord)
{
    auto output = std::ostringstream{};
    record.toStream(output);
    auto recordData = output.str();

    auto input = std::istringstream{recordData};
    auto recordSize = resultRecord.fromStream(input, recordData.size());
    ASSERT_TRUE(recordSize != 0);
    ASSERT_TRUE(record == resultRecord);
}

}


TEST(RecordSerialization, MsgBeginRequest)
{
    auto msg = std::make_unique<fcgi::MsgBeginRequest>(
                   fcgi::Role::Responder,
                   fcgi::ResultConnectionState::KeepOpen);
    auto pMsg = msg.get();
    auto record = fcgi::Record{std::move(msg), 1};
    auto resultRecord = fcgi::Record{};
    convertRecordTest(record, resultRecord);

    auto& readedMsg = resultRecord.getMessage<fcgi::MsgBeginRequest>();
    ASSERT_EQ(pMsg->role(), readedMsg.role());
    ASSERT_EQ(pMsg->resultConnectionState(), readedMsg.resultConnectionState());
}

TEST(RecordSerialization, MsgEndRequest)
{
    auto msg = std::make_unique<fcgi::MsgEndRequest>(77, fcgi::ProtocolStatus::Overloaded);
    auto pMsg = msg.get();
    auto record = fcgi::Record{std::move(msg), 1};
    auto resultRecord = fcgi::Record{};
    convertRecordTest(record, resultRecord);

    auto readedMsg = resultRecord.getMessage<fcgi::MsgEndRequest>();
    ASSERT_EQ(pMsg->appStatus(), readedMsg.appStatus());
    ASSERT_EQ(pMsg->protocolStatus(), readedMsg.protocolStatus());

}

TEST(RecordSerialization, MsgGetValues)
{
    auto msg = std::make_unique<fcgi::MsgGetValues>();
    auto pMsg = msg.get();
    auto record = fcgi::Record{std::move(msg), 1};
    auto resultRecord = fcgi::Record{};
    convertRecordTest(record, resultRecord);

    auto readedMsg = resultRecord.getMessage<fcgi::MsgGetValues>();
    ASSERT_EQ(pMsg->requestList(), readedMsg.requestList());

    msg = std::make_unique<fcgi::MsgGetValues>();
    pMsg = msg.get();
    msg->requestValue(fcgi::ValueRequest::MaxReqs);
    msg->requestValue(fcgi::ValueRequest::MpxsConns);
    {
        auto record = fcgi::Record{std::move(msg), 1};
        auto resultRecord = fcgi::Record{};
        convertRecordTest(record, resultRecord);

        readedMsg = resultRecord.getMessage<fcgi::MsgGetValues>();
        ASSERT_EQ(pMsg->requestList(), readedMsg.requestList());
    }
}

TEST(RecordSerialization, MsgGetValuesResult)
{
    auto msg = std::make_unique<fcgi::MsgGetValuesResult>();
    auto pMsg = msg.get();
    auto record = fcgi::Record{std::move(msg), 1};
    auto resultRecord = fcgi::Record{};
    convertRecordTest(record, resultRecord);

    auto readedMsg = resultRecord.getMessage<fcgi::MsgGetValuesResult>();
    ASSERT_EQ(pMsg->requestList(), readedMsg.requestList());

    msg = std::make_unique<fcgi::MsgGetValuesResult>();
    pMsg = msg.get();
    msg->setRequestValue(fcgi::ValueRequest::MaxReqs, "10");
    msg->setRequestValue(fcgi::ValueRequest::MpxsConns, "2");
    {
        auto record = fcgi::Record{std::move(msg), 1};
        auto resultRecord = fcgi::Record{};
        convertRecordTest(record, resultRecord);

        readedMsg = resultRecord.getMessage<fcgi::MsgGetValuesResult>();
        ASSERT_EQ(pMsg->requestList(), readedMsg.requestList());
        for(auto request : pMsg->requestList())
            ASSERT_EQ(pMsg->requestValue(request), readedMsg.requestValue(request));
    }
}

TEST(RecordSerialization, MsgParams)
{
    auto msg = std::make_unique<fcgi::MsgParams>();
    auto pMsg = msg.get();
    auto record = fcgi::Record{std::move(msg), 1};
    auto resultRecord = fcgi::Record{};
    convertRecordTest(record, resultRecord);

    auto readedMsg = resultRecord.getMessage<fcgi::MsgParams>();
    ASSERT_EQ(pMsg->paramList(), readedMsg.paramList());

    msg = std::make_unique<fcgi::MsgParams>();
    pMsg = msg.get();
    msg->setParam("Hello", "World");
    {
        auto record = fcgi::Record{std::move(msg), 1};
        auto resultRecord = fcgi::Record{};
        convertRecordTest(record, resultRecord);

        readedMsg = resultRecord.getMessage<fcgi::MsgParams>();
        ASSERT_EQ(pMsg->paramList(), readedMsg.paramList());
        for(const auto& param : pMsg->paramList())
            ASSERT_EQ(pMsg->paramValue(param), readedMsg.paramValue(param));
    }
}

TEST(RecordSerialization, MsgUnkownType)
{
    auto msg = std::make_unique<fcgi::MsgUnknownType>(77);
    auto pMsg = msg.get();
    auto record = fcgi::Record{std::move(msg), 1};
    auto resultRecord = fcgi::Record{};
    convertRecordTest(record, resultRecord);

    auto readedMsg = resultRecord.getMessage<fcgi::MsgUnknownType>();
    ASSERT_EQ(pMsg->unknownTypeValue(), readedMsg.unknownTypeValue());
}

TEST(RecordSerialization, StreamDataMessage)
{
    auto msg = std::make_unique<fcgi::StreamDataMessage>(fcgi::RecordType::StdIn, "Hello world");
    auto pMsg = msg.get();
    auto record = fcgi::Record{std::move(msg), 1};
    auto resultRecord = fcgi::Record{};
    convertRecordTest(record, resultRecord);

    auto readedMsg = resultRecord.getMessage<fcgi::MsgStdIn>();
    ASSERT_EQ(pMsg->data(), readedMsg.data());
}
