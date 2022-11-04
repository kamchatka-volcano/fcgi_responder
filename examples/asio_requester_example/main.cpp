#include "asio.hpp"
#include <fcgi_responder/requester.h>
#include <iostream>

using unixdomain = asio::local::stream_protocol;


class Client : public fcgi::Requester{
public:
    explicit Client(unixdomain::socket&& socket)
    : socket_(std::move(socket))
    {
    }

    void process()
    {
        while(isOpened_){
            auto receivedDataSize = socket_.read_some(asio::buffer(buffer_));
            ///
            /// Passing read socket data with fcgi::Requester::receiveData method
            ///
            receiveData(buffer_.data(), receivedDataSize);
        }
    }

private:
    ///
    /// Overriding fcgi::Requester::sendData to send request data to the FastCGI application
    ///
    void sendData(const std::string& data) override
    {
        asio::write(socket_, asio::buffer(data, data.size()));
    }

    ///
    /// Overriding fcgi::Requester::disconnect to close connection with the FastCGI application
    ///
    void disconnect() override
    {
        try{
            socket_.shutdown(unixdomain::socket::shutdown_both);
            socket_.close();
        }
        catch(const std::system_error& e){
            std::cerr << "socket close error:" << e.code();
        }
        isOpened_ = false;
    };

private:
    unixdomain::socket socket_;
    std::array<char, 65536> buffer_;
    bool isOpened_ = true;
};

void onResponseReceived(const std::optional<fcgi::ResponseData>& response)
{
    std::cout << "Response:" << std::endl;
    if (response)
        std::cout << response->data << std::endl;
    else
        std::cout << "No response" << std::endl;
}

int main ()
{
    auto socketPath = std::string{"/tmp/fcgi.sock"};
    auto io = asio::io_context{};
    auto socket = unixdomain::socket{io};
    try {
        socket.connect(unixdomain::endpoint{socketPath});
    }
    catch(std::system_error& e){
        std::cerr << "Socket connection error:" << e.code();
        return 1;
    }

    auto client = Client{std::move(socket)};
    client.setErrorInfoHandler([](const std::string& error){
        std::cout << error << std::endl;
    });
    client.sendRequest({{"REQUEST_METHOD","GET"},
                        {"REMOTE_ADDR","127.0.0.1"},
                        {"HTTP_HOST","localhost"},
                        {"REQUEST_URI","/"}}, {}, onResponseReceived);
    client.process();
    return 0;
}