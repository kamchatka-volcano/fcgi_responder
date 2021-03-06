## Overview
**fcgi_responder** is a C++17 library implementing the [Responder role](https://fast-cgi.github.io/spec#62-responder) of the FastCGI protocol.  
It works on raw data, received from the web server, and returns serialized output, required to be sent back to the server by the client.  
Thus, the implementation of connection to the web server lays outside of the library's responsibility, and clients can use whatever socket programming methods they prefer.  
This makes **fcgi_responder** being portable and having no external dependencies.  

## Usage
This is a minimal example, using the Qt framework for networking:
```C++
#include <fcgi_responder/responder.h>
#include <QCoreApplication>
#include <QObject>
#include <QLocalServer>
#include <QLocalSocket>

class FCGIGreeter : public QObject, public fcgi::Responder{
public:
    FCGIGreeter(QLocalSocket* socket, QObject* parent = nullptr)
        : QObject(parent)
        , socket_(socket)
    {
        QObject::connect(socket_, &QLocalSocket::readyRead, this, &FCGIGreeter::onReadyRead);
        QObject::connect(socket_, &QLocalSocket::disconnected, this, &FCGIGreeter::deleteLater);
    }

    ~FCGIGreeter() override
    {
        socket_->deleteLater();
    }

    void onReadyRead()
    {
        auto data = socket_->readAll();

        ///
        /// Passing readed socket data with fcgi::Responder::receiveData method
        ///
        receiveData(data.data(), static_cast<std::size_t>(data.size()));
    }

    ///
    /// Overriding fcgi::Responder::processRequest to form response data
    ///
    void processRequest(fcgi::Request&&, fcgi::Response&& response) override
    {
        response.setData("HTTP/1.1 200\r\n "
                         "Content-Type: text/html\r\n"
                         "\r\n"
                         "HELLO WORLD!");
        response.send();
    }

    ///
    /// Overriding fcgi::Responder::sendData to send response data to the web server
    ///
    void sendData(const std::string& data) override
    {
        socket_->write(data.c_str(), static_cast<qint64>(data.size()));
        socket_->flush();
    }

    ///
    /// Overriding fcgi::Responder::disonnect to close connection with the web server
    ///
    void disconnect() override
    {
        socket_->disconnectFromServer();
    }

private:
    QLocalSocket* socket_;
};

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);
    QLocalServer server;
    server.setSocketOptions(QLocalServer::WorldAccessOption);
    server.listen("/tmp/fcgicpp.sock");
    QObject::connect(&server, &QLocalServer::newConnection, [&server]()
    {
        auto socket = server.nextPendingConnection();
        new FCGIGreeter(socket, &server);
    });
    return a.exec();
}
```

## Installation
* git clone https://github.com/kamchatka-volcano/fcgi_responder.git
* cd fcgi_responder && mkdir build && cd build
* cmake ..
* make
* make install

## Running tests
* cd fcgi_responder/build
* cmake .. -DENABLE_TESTS=ON
* make
* ctest



