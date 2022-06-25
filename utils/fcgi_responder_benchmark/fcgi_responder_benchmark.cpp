#include "asio.hpp"
#include <fcgi_responder/responder.h>
#include <cmdlime/gnuconfig.h>
#include <iostream>

using unixdomain = asio::local::stream_protocol;

std::string generateText(std::size_t sizeKb)
{
    auto res = std::string{};
    for (auto i = 0u; i < sizeKb; ++i)
        res += "Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Aenean commodo ligula eget dolor. Aenean massa. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Donec quam felis, ultricies nec, pellentesque eu, pretium quis, sem. Nulla consequat massa quis enim. Donec pede justo, fringilla vel, aliquet nec, vulputate eget, arcu. In enim justo, rhoncus ut, imperdiet a, venenatis vitae, justo. Nullam dictum felis eu pede mollis pretium. Integer tincidunt. Cras dapibus. Vivamus elementum semper nisi. Aenean vulputate eleifend tellus. Aenean leo ligula, porttitor eu, consequat vitae, eleifend ac, enim. Aliquam lorem ante, dapibus in, viverra quis, feugiat a, tellus. Phasellus viverra nulla ut metus varius laoreet. Quisque rutrum. Aenean imperdiet. Etiam ultricies nisi vel augue. Curabitur ullamcorper ultricies nisi. Nam eget dui. Etiam rhoncus. Maecenas tempus, tellus eget condimentum rhoncus, sem quam semper libero, sit amet adipiscing sem neque sed ipsum. Nam quam nunc, blandit  \n";
    return res;
}

class Connection : public fcgi::Responder{
public:
    explicit Connection(int responseSize, unixdomain::socket&& socket)
    : socket_{std::move(socket)}
    , responseSize_{responseSize}
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
            } catch(...){
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
        static auto payload = generateText(responseSize_);
        response.setData("HTTP/1.1 200 OK\r\n "
                         "Content-Type: text/html\r\n"
                         "\r\n"
                         + payload);
        response.send();
    }

private:
    unixdomain::socket socket_;
    int responseSize_;
    std::array<char, 65536> buffer_;
    bool isOpened_ = true;
};

struct Cfg : public cmdlime::GNUConfig{
    CMDLIME_PARAM(responseSize, int) << "response size in kb"
                                     << [](int value) {
                                          if (value < 1)
                                              throw cmdlime::ValidationError{"response size must be greater than 0"};
                                     };
};

int main (int argc, char** argv)
{
    auto cfg = Cfg{};
    auto configReader = cmdlime::ConfigReader{cfg, "fcgi_responder_benchmark"};
    if (!configReader.readCommandLine(argc, argv))
        return configReader.exitCode();

    auto socketPath = std::string{"/tmp/fcgi.sock"};
    umask(0);
    chmod(socketPath.c_str(), 0777);
    unlink(socketPath.c_str());

    auto io = asio::io_context{};
    auto acceptor = unixdomain::acceptor{io, unixdomain::endpoint{socketPath}};

    while (true) {
        auto socket = acceptor.accept();
        auto connection = Connection{cfg.responseSize, std::move(socket)};
        connection.process();
    }
    return 0;
}
