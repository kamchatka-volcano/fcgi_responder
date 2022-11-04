#include <fcgi_responder/requester.h>
#include <QCoreApplication>
#include <QObject>
#include <QLocalSocket>

class Client : public QObject, public fcgi::Requester{
public:
    Client(QLocalSocket& socket)
        : socket_(socket)
    {
        QObject::connect(&socket_, &QLocalSocket::readyRead, this, &Client::onReadyRead);
    }

    void onReadyRead()
    {
        auto data = socket_.readAll();

        ///
        /// Passing read socket data with fcgi::Requester::receiveData method
        ///
        receiveData(data.data(), static_cast<std::size_t>(data.size()));
    }

    ///
    /// Overriding fcgi::Requester::sendData to send response data to the FastCGI application
    ///
    void sendData(const std::string& data) override
    {
        socket_.write(data.c_str(), static_cast<qint64>(data.size()));
        socket_.flush();
    }

    ///
    /// Overriding fcgi::Requester::disconnect to close connection with the FastCGI application
    ///
    void disconnect() override
    {
        socket_.disconnectFromServer();
    }



private:
    QLocalSocket& socket_;
};

void onResponseReceived(const std::optional<fcgi::ResponseData>& response)
{
    qDebug() << "Response:";
    if (response)
        qDebug() << QString::fromStdString(response->data);
    else
        qDebug() << "No response";
}

int main(int argc, char* argv[])
{
    auto a = QCoreApplication{argc, argv};
    auto socket = QLocalSocket{};
    socket.connectToServer("/tmp/fcgi.sock");
    if (!socket.waitForConnected()) {
        qDebug() << "Socket connection error: " << socket.errorString();
        return 1;
    }
    auto client = Client{socket};
    client.setErrorInfoHandler([](const std::string& error){
        qDebug() << QString::fromStdString(error);
    });
    client.sendRequest({{"REQUEST_METHOD","GET"},
                        {"REMOTE_ADDR","127.0.0.1"},
                        {"HTTP_HOST","localhost"},
                        {"REQUEST_URI","/"}}, {}, onResponseReceived);
    return a.exec();
}