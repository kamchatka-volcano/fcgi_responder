#include "asio.hpp"
#include <fcgi_responder/responder.h>
#include <iostream>

using unixdomain = asio::local::stream_protocol;

class Connection : public fcgi::Responder{
public:
    explicit Connection(unixdomain::socket&& socket)
    : socket_(std::move(socket))
    {
    }

    void process()
    {
        while(isOpened_){
            try {
                auto receivedDataSize = socket_.read_some(asio::buffer(buffer_));
                ///
                /// Passing read socket data with fcgi::Responder::receiveData method
                ///
                receiveData(buffer_.data(), receivedDataSize);
            }
            catch(...){
                isOpened_ = false;
                return;
            }
        }
    }

private:
    ///
    /// Overriding fcgi::Responder::sendData to send response data to the web server
    ///
    void sendData(const std::string& data) override
    {
        asio::write(socket_, asio::buffer(data, data.size()));
    }

    ///
    /// Overriding fcgi::Responder::disconnect to close connection with the web server
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

    ///
    /// Overriding fcgi::Responder::processRequest to form response data
    ///
    void processRequest(fcgi::Request&&, fcgi::Response&& response) override
    {
        response.setData("HTTP/1.1 200 OK\r\n "
                         "Content-Type: text/html\r\n"
                         "\r\n"
                         "HELLO WORLD USING ASIO!");
        response.send();
    }

private:
    unixdomain::socket socket_;
    std::array<char, 65536> buffer_;
    bool isOpened_ = true;
};

int main ()
{
    auto socketPath = std::string{"/tmp/fcgi.sock"};
    umask(0);
    chmod(socketPath.c_str(), 0777);
    unlink(socketPath.c_str());

    auto io = asio::io_context{};
    auto acceptor = unixdomain::acceptor{io, unixdomain::endpoint{socketPath}};

    while (true) {
        auto socket = acceptor.accept();
        auto connection = Connection{std::move(socket)};
        connection.process();
    }
    return 0;
}
