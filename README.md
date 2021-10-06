## Overview
**fcgi_responder** is a C++17 library implementing the [Responder role](https://fast-cgi.github.io/spec#62-responder) of the FastCGI protocol.  
It works on raw data, received from the web server, and returns serialized output, required to be sent back to the server by the client.  
Thus, the implementation of connection to the web server lays outside of the library's responsibility, and clients can use whatever socket programming methods they prefer.  
This makes **fcgi_responder** being portable and having no external dependencies.  

## Usage
Inherit from `fcgi::Responder` class and implement its pure virtual methods to provide necessary functionality:  
    * `virtual void sendData(const std::string& data) = 0`   
    * `virtual void processRequest(fcgi::Request&& request, fcgi::Response&& response) = 0`  
    * `virtual void disconnect() = 0`

Then, listen for connections from the web server and process the incoming data by passing it to the `fcgi::Responder::receiveData` method.

This is a minimal example, using the standalone asio library for networking:
```C++
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
            auto receivedDataSize = socket_.read_some(asio::buffer(buffer_));
            ///
            /// Passing read socket data with fcgi::Responder::receiveData method
            ///
            receiveData(buffer_.data(), receivedDataSize);
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
    /// Overriding fcgi::Responder::disonnect to close connection with the web server
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
    void processRequest(fcgi::Request&& request, fcgi::Response&& response) override
    {
        response.setData("HTTP/1.1 200\r\n "
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
```
Check the `examples` directory for this and other example using the Qt framework.

## Installation
Download and link the library from your project's CMakeLists.txt:
```
cmake_minimum_required(VERSION 3.14)

include(FetchContent)

FetchContent_Declare(fcgi_responder
    GIT_REPOSITORY "https://github.com/kamchatka-volcano/fcgi_responder.git"
    GIT_TAG "origin/master"
)
FetchContent_MakeAvailable(fcgi_responder)

add_executable(${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} PRIVATE fcgi_responder)
```

For the system-wide installation use these commands:
```
git clone https://github.com/kamchatka-volcano/fcgi_responder.git
cd fcgi_responder
cmake -S . -B build
cmake --build build
cmake --install build
```

## Running tests
```
cd fcgi_responder
cmake -S . -B build -DENABLE_TESTS=ON
cmake --build build 
cd build/tests && ctest
```

## Running examples
Set up your webserver to use the FastCGI protocol over unix domain socket `/tmp/fcgi.sock`. With NGINX you can use this config:

```
server {
	listen 8088;
	server_name localhost;
	index /~;

	location / {
		try_files $uri $uri/ @fcgi;
	}
	
	location @fcgi {
		fastcgi_pass  unix:/tmp/fcgi.sock;
		include fastcgi_params;
		fastcgi_intercept_errors on;
		fastcgi_keep_conn off;
	}
}

```

Build and run asio example:

```
cd fcgi_responder
cmake -S . -B build -DENABLE_ASIO_EXAMPLE=ON
cmake --build build 
./build/examples/using-asio/asio_example
```

Or build and run Qt example:

```
cd fcgi_responder
cmake -S . -B build -DENABLE_QT_EXAMPLE=ON
cmake --build build 
./build/examples/using-qt/qt_example
```

Check that it's working here: http://localhost:8088 


### License
**fcgi_responder** is licensed under the [MS-PL license](/LICENSE.md)  


