#include <responder.h>
#include <record.h>
#include <msgbeginrequest.h>
#include <msgendrequest.h>
#include <msggetvalues.h>
#include <msggetvaluesresult.h>
#include <msgunknowntype.h>
#include <msgparams.h>
#include <msgabortrequest.h>
#include <streamdatamessage.h>
#include <encoder.h>
#include <constants.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace fcgi;

namespace {
Request makeRequest(const MsgParams& params, const MsgStdIn& data){
    auto requestParams = std::vector<std::pair<std::string, std::string>>{};
    for (const auto& paramName : params.paramList())
        requestParams.emplace_back(paramName, params.paramValue(paramName));

    return Request{std::move(requestParams), std::string{data.data()}};
}
}

class MockResponder : public Responder{
public:
    MOCK_METHOD1(sendData, void(const std::string& data));
    MOCK_METHOD0(disconnect, void());
    MOCK_METHOD1(doProcessRequest, void(const Request& request));
    void processRequest(Request&& request, Response&&) override
    {
        doProcessRequest(request);
    }

    void receive(const std::string& data)
    {
        Responder::receiveData(data.c_str(), data.size());
    }
};

class MockResponderWithTestProcessor : public Responder{
public:
    MOCK_METHOD1(sendData, void(const std::string& data));
    MOCK_METHOD0(disconnect, void());
    void processRequest(Request&& request, Response&& response) override
    {
        auto testMsg = request.stdIn();
        std::reverse(testMsg.begin(), testMsg.end());
        response.setData(testMsg);
    }
    void receive(const std::string& data)
    {
        Responder::receiveData(data.c_str(), data.size());
    }
};

namespace{
    template<typename... Args>
    std::string messageData(Args&&... args)
    {
        auto record = fcgi::Record{std::forward<Args>(args)...};
        auto recordStream = std::ostringstream{};
        record.toStream(recordStream);
        return recordStream.str();
    }
}

template <typename ResponderType>
class BaseTestResponder : public ::testing::TestWithParam<bool>{
public:
    BaseTestResponder()
    {
        responder_.setErrorInfoHandler([&errorInfo = errorInfo_](const std::string& errorMsg){
            errorInfo += errorMsg + "\n";
        });
    }
protected:
    void receiveData(const std::string& data)
    {
        responder_.receive(data);
    }
    template <typename TMsg, std::enable_if_t<!std::is_convertible_v<TMsg, std::string>>* = nullptr>
    void receiveMessage(TMsg&& msg, uint16_t requestId = 0)
    {
        responder_.receive(messageData(std::forward<TMsg>(msg), requestId));
    }
    void receiveMessage(const std::string& recordData)
    {
        responder_.receive(recordData);
    }
    template <typename TMsg>
    void expectMessageToBeSent(TMsg&& msg, uint16_t requestId = 0)
    {
        EXPECT_CALL(responder_, sendData(messageData(std::forward<TMsg>(msg), requestId)));
    }
    void expectNoMessagesToBeSent()
    {
        EXPECT_CALL(responder_, sendData(::testing::_)).Times(0);
    }
    void checkConnectionState()
    {
        auto disconnectOnEnd = GetParam();
        if (disconnectOnEnd)
            EXPECT_CALL(responder_, disconnect());
        else
            EXPECT_CALL(responder_, disconnect()).Times(0);
    }
    ResultConnectionState resultConnectionState()
    {
        auto disconnectOnEnd = GetParam();
        return disconnectOnEnd ? ResultConnectionState::Close
                               : ResultConnectionState::KeepOpen;
    }

    ResponderType responder_;
    std::string errorInfo_;
};

using TestResponder = BaseTestResponder<MockResponder>;
using TestResponderWithTestProcessor = BaseTestResponder<MockResponderWithTestProcessor>;

TEST_F(TestResponder, UnknownType)
{
    expectMessageToBeSent(MsgUnknownType{99}, 0);

    auto output = std::ostringstream{};
    auto encoder = Encoder(output);
    encoder  << hardcoded::protocolVersion
             << static_cast<uint8_t>(99)
             << static_cast<uint16_t>(1)
             << static_cast<uint16_t>(0)
             << static_cast<uint8_t>(0);
    encoder.addPadding(1);
    receiveMessage(output.str());
    EXPECT_EQ(errorInfo_, "Record type \"99\" is invalid.\n");
}

TEST_F(TestResponder, GetValuesAllDefaultSettings)
{
    auto resultMsg = MsgGetValuesResult{};
    resultMsg.setRequestValue(ValueRequest::MaxReqs, std::to_string(responder_.maximumRequestsNumber()));
    resultMsg.setRequestValue(ValueRequest::MaxConns, std::to_string(responder_.maximumConnectionsNumber()));
    resultMsg.setRequestValue(ValueRequest::MpxsConns, std::to_string(responder_.isMultiplexingEnabled() ? 1 : 0));
    expectMessageToBeSent(std::move(resultMsg));

    auto requestMsg = MsgGetValues{};
    requestMsg.requestValue(ValueRequest::MaxReqs);
    requestMsg.requestValue(ValueRequest::MaxConns);
    requestMsg.requestValue(ValueRequest::MpxsConns);
    receiveMessage(std::move(requestMsg));
}

TEST_F(TestResponder, GetValueChangedSettings)
{
    responder_.setMaximumRequestsNumber(99);
    auto resultMsg = MsgGetValuesResult{};
    resultMsg.setRequestValue(ValueRequest::MaxReqs, std::to_string(responder_.maximumRequestsNumber()));
    resultMsg.setRequestValue(ValueRequest::MpxsConns, std::to_string(responder_.isMultiplexingEnabled() ? 1 : 0));
    expectMessageToBeSent(std::move(resultMsg));

    auto requestMsg = MsgGetValues{};
    requestMsg.requestValue(ValueRequest::MaxReqs);
    requestMsg.requestValue(ValueRequest::MpxsConns);
    receiveMessage(std::move(requestMsg));
}

TEST_P(TestResponder, AbortRequest)
{
    expectMessageToBeSent(MsgEndRequest{0, ProtocolStatus::RequestComplete}, 1);
    checkConnectionState();

    receiveMessage(MsgBeginRequest{Role::Responder, resultConnectionState()}, 1);
    receiveMessage(RecordType::AbortRequest, 1);
}

TEST_P(TestResponder, UnknownRole)
{
    expectMessageToBeSent(MsgEndRequest{0, ProtocolStatus::UnknownRole});
    checkConnectionState();

    receiveMessage(MsgBeginRequest{Role::Authorizer, resultConnectionState()});
}

TEST_P(TestResponder, CantMultiplex)
{
    responder_.setMultiplexingEnabled(false);
    expectMessageToBeSent(MsgEndRequest{0, ProtocolStatus::CantMpxConn}, 2);
    checkConnectionState();

    receiveMessage(MsgBeginRequest{Role::Responder, resultConnectionState()}, 1);
    receiveMessage(MsgBeginRequest{Role::Responder, resultConnectionState()}, 2);
}

TEST_P(TestResponder, Overloaded)
{
    responder_.setMaximumRequestsNumber(2);
    expectMessageToBeSent(MsgEndRequest{0, ProtocolStatus::Overloaded}, 3);
    checkConnectionState();

    receiveMessage(MsgBeginRequest{Role::Responder, resultConnectionState()}, 1);
    receiveMessage(MsgBeginRequest{Role::Responder, resultConnectionState()}, 2);
    receiveMessage(MsgBeginRequest{Role::Responder, resultConnectionState()}, 3);
}

namespace fcgi{
bool operator==(const Request& lhs, const Request& rhs)
{
    return lhs.params() == rhs.params() &&
           lhs.stdIn() == rhs.stdIn();
}
}

TEST_P(TestResponder, Request)
{
    auto params = MsgParams{};
    params.setParam("test", "hello world");
    params.setParam("foo", "bar");

    auto inStream = MsgStdIn{"HELLO WORLD"};

    auto expectedRequest = makeRequest(params, inStream);
    ::testing::InSequence seq;
    EXPECT_CALL(responder_, doProcessRequest(expectedRequest));
    expectMessageToBeSent(MsgStdOut{}, 1);
    expectMessageToBeSent(MsgStdErr{}, 1);
    expectMessageToBeSent(MsgEndRequest{0, ProtocolStatus::RequestComplete}, 1);
    checkConnectionState();

    receiveMessage(MsgBeginRequest{Role::Responder, resultConnectionState()}, 1);
    receiveMessage(std::move(params), 1);
    receiveMessage(MsgParams{}, 1);
    receiveMessage(std::move(inStream), 1);
    receiveMessage(MsgStdIn{}, 1);
}

TEST_P(TestResponder, ReceivingMessagesInLargeChunks)
{
    auto streamRecordData = std::string(hardcoded::maxDataMessageSize, '0');
    auto streamData = std::string{};
    auto expectedRequestData = std::string{};
    auto requestId = static_cast<uint16_t>(1);
    for (auto i = 0; i < 3; ++i){
        auto inStream = MsgStdIn{streamRecordData};
        expectedRequestData += inStream.data();
        streamData += messageData(std::move(inStream), requestId);
    }
    streamData += messageData(MsgStdIn{}, requestId);

    auto expectedRequest = Request{{}, expectedRequestData};
    ::testing::InSequence seq;
    EXPECT_CALL(responder_, doProcessRequest(expectedRequest));
    expectMessageToBeSent(MsgStdOut{}, requestId);
    expectMessageToBeSent(MsgStdErr{}, requestId);
    expectMessageToBeSent(MsgEndRequest{0, ProtocolStatus::RequestComplete}, requestId);
    checkConnectionState();

    receiveMessage(MsgBeginRequest{Role::Responder, resultConnectionState()}, requestId);
    auto halfSize = streamData.size() / 2;
    receiveData(streamData.substr(0, halfSize));
    receiveData(streamData.substr(halfSize, streamData.size() - halfSize));
}

TEST_F(TestResponder, Multiplexing)
{
    auto params = MsgParams{};
    params.setParam("test", "hello world");
    params.setParam("foo", "bar");

    auto inStream = MsgStdIn{"HELLO WORLD"};

    auto params2 = MsgParams{};
    params2.setParam("msg", "param");

    auto expectedRequest = makeRequest(params, inStream);
    auto expectedRequest2 = makeRequest(params2, {});

    ::testing::InSequence seq;
    EXPECT_CALL(responder_, doProcessRequest(expectedRequest2));
    expectMessageToBeSent(MsgStdOut{}, 2);
    expectMessageToBeSent(MsgStdErr{}, 2);
    expectMessageToBeSent(MsgEndRequest{0, ProtocolStatus::RequestComplete}, 2);
    EXPECT_CALL(responder_, doProcessRequest(expectedRequest));
    expectMessageToBeSent(MsgStdOut{}, 1);
    expectMessageToBeSent(MsgStdErr{}, 1);
    expectMessageToBeSent(MsgEndRequest{0, ProtocolStatus::RequestComplete}, 1);
    EXPECT_CALL(responder_, disconnect()).Times(0);

    receiveMessage(MsgBeginRequest{Role::Responder, ResultConnectionState::KeepOpen}, 1);
    receiveMessage(std::move(params), 1);
    receiveMessage(MsgBeginRequest{Role::Responder, ResultConnectionState::KeepOpen}, 2);
    receiveMessage(std::move(params2), 2);
    receiveMessage(MsgParams{}, 1);
    receiveMessage(std::move(inStream), 1);
    receiveMessage(MsgParams{}, 2);
    receiveMessage(MsgStdIn{}, 2);
    receiveMessage(MsgStdIn{}, 1);
}

TEST_P(TestResponderWithTestProcessor, Request)
{
    auto params = MsgParams{};
    params.setParam("test", "hello world");
    params.setParam("foo", "bar");

    auto inStream = MsgStdIn{"HELLO WORLD"};

    ::testing::InSequence seq;
    expectMessageToBeSent(MsgStdOut{"DLROW OLLEH"}, 1);
    expectMessageToBeSent(MsgStdOut{}, 1);
    expectMessageToBeSent(MsgStdErr{}, 1);
    expectMessageToBeSent(MsgEndRequest{0, ProtocolStatus::RequestComplete}, 1);
    checkConnectionState();

    receiveMessage(MsgBeginRequest{Role::Responder, resultConnectionState()}, 1);
    receiveMessage(std::move(params), 1);
    receiveMessage(MsgParams{}, 1);
    receiveMessage(std::move(inStream), 1);
    receiveMessage(MsgStdIn{}, 1);
}

TEST_F(TestResponder, UnexpectedRecord)
{
    expectNoMessagesToBeSent();
    receiveMessage(MsgAbortRequest{}, 1);
    EXPECT_EQ(errorInfo_, "Received unexpected record, RecordType = 2, requestId = 1\n");
}

TEST_P(TestResponder, RecordReadError)
{
    expectMessageToBeSent(MsgEndRequest{0, ProtocolStatus::RequestComplete}, 1);
    checkConnectionState();

    auto output = std::ostringstream{};
    auto encoder = Encoder{output};
    auto nameValue = fcgi::NameValue{"wrongName", "0"};
    encoder  << hardcoded::protocolVersion
             << static_cast<uint8_t>(RecordType::GetValues)
             << static_cast<uint16_t>(1)
             << static_cast<uint16_t>(nameValue.size())
             << static_cast<uint8_t>(0);
    encoder.addPadding(1);
    nameValue.toStream(output);

    auto receivedData = output.str();
    receivedData += messageData(MsgBeginRequest{Role::Responder, resultConnectionState()}, static_cast<uint16_t>(1));
    receivedData += messageData(MsgAbortRequest{}, static_cast<uint16_t>(1));
    receiveMessage(receivedData);

    EXPECT_EQ(errorInfo_, "Record message read error: Value request value \"wrongName\" is invalid.\n");
}

TEST_P(TestResponder, RecordReadErrorMisalignedNameValue)
{
    expectNoMessagesToBeSent();
    auto output = std::ostringstream{};
    auto encoder = Encoder{output};
    auto nameValue = fcgi::NameValue{"FCGI_MAX_CONNS", ""};
    encoder  << hardcoded::protocolVersion
             << static_cast<uint8_t>(RecordType::GetValues)
             << static_cast<uint16_t>(1)
             << static_cast<uint16_t>(nameValue.size() - 1) //wrong size
             << static_cast<uint8_t>(0);
    encoder.addPadding(1);
    nameValue.toStream(output);

    auto receivedData = output.str();
    receivedData += messageData(MsgBeginRequest{Role::Responder, resultConnectionState()}, static_cast<uint16_t>(1));
    receivedData += messageData(MsgAbortRequest{}, static_cast<uint16_t>(1));
    receiveMessage(receivedData);
    EXPECT_EQ(errorInfo_, "Record message read error: Misaligned name-value\nProtocol version \"83\" isn't supported.\n");
}

INSTANTIATE_TEST_SUITE_P(WithConnectionStateCheck, TestResponder, ::testing::Values(false, true));
INSTANTIATE_TEST_SUITE_P(WithConnectionStateCheck, TestResponderWithTestProcessor, ::testing::Values(false, true));

