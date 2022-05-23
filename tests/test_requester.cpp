#include <requester.h>
#include <record.h>
#include <request.h>
#include <msgbeginrequest.h>
#include <msgendrequest.h>
#include <msggetvalues.h>
#include <msggetvaluesresult.h>
#include <msgunknowntype.h>
#include <msgparams.h>
#include <msgabortrequest.h>
#include <streamdatamessage.h>
#include <constants.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace fcgi;
using namespace testing;

class MockRequester : public Requester{
public:
    MOCK_METHOD1(sendData, void(const std::string& data));
    MOCK_METHOD1(onResponseReceived, void(const std::optional<ResponseData>& response));
    MOCK_METHOD0(disconnect, void());
    void receive(const std::string& data)
    {
        Requester::receiveData(data.c_str(), data.size());
    }
    std::optional<fcgi::RequestHandle> send(
            const std::map<std::string, std::string>& fcgiParams,
            const std::string& fcgiData)
    {
        return Requester::sendRequest(fcgiParams, fcgiData, [this](const std::optional<ResponseData>& response){
            onResponseReceived(response);
        });
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

namespace fcgi{
bool operator==(const ResponseData& lhs, const ResponseData& rhs)
{
    return lhs.data == rhs.data && lhs.errorMsg == rhs.errorMsg;
}
}

class TestRequester : public TestWithParam<bool>{
public:
    TestRequester()
    {
        requester_.setErrorInfoHandler([&errorInfo = errorInfo_](const std::string& errorMsg){
            errorInfo += errorMsg + "\n";
        });
    }
protected:
    void receiveData(const std::string& data)
    {
        requester_.receive(data);
    }
    template <typename TMsg, std::enable_if_t<!std::is_convertible_v<TMsg, std::string>>* = nullptr>
    void receiveMessage(TMsg&& msg, uint16_t requestId = 0)
    {
        requester_.receive(messageData(std::forward<TMsg>(msg), requestId));
    }
    void receiveMessage(const std::string& recordData)
    {
        requester_.receive(recordData);
    }
    template <typename TMsg>
    void expectMessageToBeSent(TMsg&& msg, uint16_t requestId = 0)
    {
        EXPECT_CALL(requester_, sendData(messageData(std::forward<TMsg>(msg), requestId)));
    }
    void expectNoMessagesToBeSent()
    {
        EXPECT_CALL(requester_, sendData(::testing::_)).Times(0);
    }
    void expectReceiveResponse(const std::optional<ResponseData>& response)
    {
        EXPECT_CALL(requester_, onResponseReceived(response));
        if (!keepConnection_)
            EXPECT_CALL(requester_, disconnect());
    }

    void initConnection(int maxRequestsNumber = 10, bool isMultiplexingEnabled = true)
    {
        auto msgGetValues = MsgGetValues{};
        msgGetValues.requestValue(ValueRequest::MaxReqs);
        msgGetValues.requestValue(ValueRequest::MpxsConns);
        expectMessageToBeSent(msgGetValues);

        auto msgGetValuesResult = MsgGetValuesResult{};
        msgGetValuesResult.setRequestValue(ValueRequest::MaxReqs, std::to_string(maxRequestsNumber));
        msgGetValuesResult.setRequestValue(ValueRequest::MpxsConns, std::to_string(isMultiplexingEnabled ? 1 : 0));
        receiveMessage(msgGetValuesResult);
    }

    std::optional<fcgi::RequestHandle> makeRequest(
            const std::map<std::string, std::string>& fcgiParams,
            const std::string& fcgiData,
            uint16_t requestId = 1,
            int maxRequestsNumber = 10,
            bool isMultiplexingEnabled = true)
    {
        if (firstRequest_){
            auto msgGetValues = MsgGetValues{};
            msgGetValues.requestValue(ValueRequest::MaxReqs);
            msgGetValues.requestValue(ValueRequest::MpxsConns);
            expectMessageToBeSent(msgGetValues);
        }
        expectMessageToBeSent(MsgBeginRequest{Role::Responder, keepConnection_ ? ResultConnectionState::KeepOpen
                                                                               : ResultConnectionState::Close},
                              requestId);

        auto paramsMsg = MsgParams{};
        for (const auto& [paramName, paramValue] : fcgiParams)
            paramsMsg.setParam(paramName, paramValue);
        expectMessageToBeSent(paramsMsg, requestId);
        if (!fcgiParams.empty())
            expectMessageToBeSent(MsgParams{}, requestId);
        expectMessageToBeSent(MsgStdIn{fcgiData}, requestId);
        if (!fcgiData.empty())
            expectMessageToBeSent(MsgStdIn{""}, requestId);

        auto requestHandle = requester_.send(fcgiParams, fcgiData);
        if (firstRequest_) {
            auto msgGetValuesResult = MsgGetValuesResult{};
            msgGetValuesResult.setRequestValue(ValueRequest::MaxReqs, std::to_string(maxRequestsNumber));
            msgGetValuesResult.setRequestValue(ValueRequest::MpxsConns, std::to_string(isMultiplexingEnabled ? 1 : 0));
            receiveMessage(msgGetValuesResult);
        }
        firstRequest_ = false;
        return requestHandle;
    }

    MockRequester requester_;
    std::string errorInfo_;
    bool keepConnection_ = false;
    bool firstRequest_ = true;
};

TEST_P(TestRequester, UnknownType)
{
    const auto seq = InSequence{};
    const auto requestId = 1;

    makeRequest({}, "", requestId);

    expectMessageToBeSent(MsgAbortRequest{}, requestId);
    auto msgUnknownType = MsgUnknownType{77};;
    receiveMessage(msgUnknownType, requestId);
}

TEST_P(TestRequester, NoResponseOverloaded)
{
    const auto seq = InSequence{};
    const auto requestId = 1;
    makeRequest({}, "", requestId);
    expectReceiveResponse(std::nullopt);
    receiveMessage(MsgStdOut{}, requestId);
    receiveMessage(MsgStdErr{}, requestId);
    receiveMessage(MsgEndRequest{0, ProtocolStatus::Overloaded}, requestId);
}

TEST_P(TestRequester, NoResponseCantMultiplex)
{
    const auto seq = InSequence{};
    const auto requestId = 1;
    makeRequest({}, "", requestId);
    expectReceiveResponse(std::nullopt);
    receiveMessage(MsgStdOut{}, requestId);
    receiveMessage(MsgStdErr{}, requestId);
    receiveMessage(MsgEndRequest{0, ProtocolStatus::CantMpxConn}, requestId);
}

TEST_P(TestRequester, NoResponseUnknownRole)
{
    const auto seq = InSequence{};
    const auto requestId = 1;
    makeRequest({}, "", requestId);
    expectReceiveResponse(std::nullopt);
    receiveMessage(MsgStdOut{}, requestId);
    receiveMessage(MsgStdErr{}, requestId);
    receiveMessage(MsgEndRequest{0, ProtocolStatus::UnknownRole}, requestId);
}

TEST_P(TestRequester, EmptyResponse)
{
    const auto seq = InSequence{};
    const auto requestId = 1;
    makeRequest({}, "", requestId);
    expectReceiveResponse(ResponseData{"", ""});
    receiveMessage(MsgStdOut{}, requestId);
    receiveMessage(MsgStdErr{}, requestId);
    receiveMessage(MsgEndRequest{0, ProtocolStatus::RequestComplete}, requestId);
}

TEST_P(TestRequester, ConnectionSettings)
{
    const auto seq = InSequence{};
    const auto requestId = 1;
    makeRequest({}, "", requestId, 11, false);
    expectReceiveResponse(ResponseData{"", ""});
    receiveMessage(MsgStdOut{}, requestId);
    receiveMessage(MsgStdErr{}, requestId);
    receiveMessage(MsgEndRequest{0, ProtocolStatus::RequestComplete}, requestId);
    EXPECT_EQ(requester_.maximumRequestsNumber(), 11);
    EXPECT_EQ(requester_.isMultiplexingEnabled(), false);
}

TEST_P(TestRequester, ResponseData)
{
    const auto seq = InSequence{};
    const auto requestId = 1;
    makeRequest({}, "", requestId);
    expectReceiveResponse(ResponseData{"Hello world", "error#1"});
    receiveMessage(MsgStdOut{"Hello world"}, requestId);
    receiveMessage(MsgStdOut{}, requestId);
    receiveMessage(MsgStdErr{"error#1"}, requestId);
    receiveMessage(MsgStdErr{}, requestId);
    receiveMessage(MsgEndRequest{0, ProtocolStatus::RequestComplete}, requestId);
}

TEST_P(TestRequester, ResponseDataMultiRequests)
{
    const auto seq = InSequence{};
    const auto requestId = 1;
    const auto secondRequestId = 2;
    makeRequest({}, "", requestId);
    EXPECT_EQ(requester_.availableRequestsNumber(), 9);
    makeRequest({}, "", secondRequestId);
    EXPECT_EQ(requester_.availableRequestsNumber(), 8);
    expectReceiveResponse(ResponseData{"Hello world", "error#1"});
    expectReceiveResponse(ResponseData{"FooBar", ""});
    receiveMessage(MsgStdOut{"Hello world"}, requestId);
    receiveMessage(MsgStdOut{"Foo"}, secondRequestId);
    receiveMessage(MsgStdOut{}, requestId);
    receiveMessage(MsgStdErr{"error#1"}, requestId);
    receiveMessage(MsgStdOut{"Bar"}, secondRequestId);
    receiveMessage(MsgStdOut{}, secondRequestId);
    receiveMessage(MsgStdErr{}, requestId);
    receiveMessage(MsgEndRequest{0, ProtocolStatus::RequestComplete}, requestId);
    EXPECT_EQ(requester_.availableRequestsNumber(), 9);
    receiveMessage(MsgEndRequest{0, ProtocolStatus::RequestComplete}, secondRequestId);
    EXPECT_EQ(requester_.availableRequestsNumber(), 10);
}

TEST_P(TestRequester, CancelRequestDuringConnection)
{
    const auto seq = InSequence{};
    auto msgGetValues = MsgGetValues{};
    msgGetValues.requestValue(ValueRequest::MaxReqs);
    msgGetValues.requestValue(ValueRequest::MpxsConns);
    expectMessageToBeSent(msgGetValues);
    auto requestHandle = requester_.send({}, "");
    EXPECT_EQ(requester_.availableRequestsNumber(), 0);
    EXPECT_CALL(requester_, onResponseReceived(std::optional<ResponseData>{}));
    requestHandle->cancelRequest();
    EXPECT_EQ(requester_.availableRequestsNumber(), 1);
}

TEST_P(TestRequester, CancelRequestAfterConnection)
{
    const auto seq = InSequence{};
    const auto requestId = 1;

    auto requestHandle = makeRequest({}, "", requestId);
    EXPECT_EQ(requester_.availableRequestsNumber(), 9);
    EXPECT_CALL(requester_, onResponseReceived(std::optional<ResponseData>{}));
    requestHandle->cancelRequest();
    EXPECT_EQ(requester_.availableRequestsNumber(), 10);
}

TEST_P(TestRequester, CancelRequestDuringResponse)
{
    const auto seq = InSequence{};
    const auto requestId = 1;
    auto requestHandle = makeRequest({}, "", requestId);
    EXPECT_EQ(requester_.availableRequestsNumber(), 9);
    EXPECT_CALL(requester_, onResponseReceived(std::optional<ResponseData>{}));
    receiveMessage(MsgStdOut{"Hello world"}, requestId);
    requestHandle->cancelRequest();
    EXPECT_EQ(requester_.availableRequestsNumber(), 10);
}

TEST_P(TestRequester, RequestExceedsMaxNumber)
{
    const auto seq = InSequence{};
    const auto requestId = 1;
    const auto maxRequestsNumber = 1;
    makeRequest({}, "", requestId, maxRequestsNumber);
    EXPECT_EQ(requester_.availableRequestsNumber(), 0);
    EXPECT_CALL(requester_, onResponseReceived(std::optional<ResponseData>{}));
    auto requestHandle =  requester_.send({}, "");
    EXPECT_EQ(requestHandle, std::nullopt);
    expectReceiveResponse(ResponseData{"Hello world", "error#1"});
    receiveMessage(MsgStdOut{"Hello world"}, requestId);
    receiveMessage(MsgStdOut{}, requestId);
    receiveMessage(MsgStdErr{"error#1"}, requestId);
    receiveMessage(MsgStdErr{}, requestId);
    receiveMessage(MsgEndRequest{0, ProtocolStatus::RequestComplete}, requestId);
    EXPECT_EQ(requester_.availableRequestsNumber(), 1);
}

INSTANTIATE_TEST_SUITE_P(TestRequester, TestRequester, Values(false, true));