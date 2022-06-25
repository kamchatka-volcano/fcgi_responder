#include <cmdlime/gnuconfig.h>
#include <fcgiapp.h>
#include <fcgio.h>
#include <sys/stat.h>
#include <ostream>
#include <functional>
#include <string_view>

std::string generateText(std::size_t sizeKb)
{
    auto res = std::string{};
    for (auto i = 0u; i < sizeKb; ++i)
        res += "Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Aenean commodo ligula eget dolor. Aenean massa. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Donec quam felis, ultricies nec, pellentesque eu, pretium quis, sem. Nulla consequat massa quis enim. Donec pede justo, fringilla vel, aliquet nec, vulputate eget, arcu. In enim justo, rhoncus ut, imperdiet a, venenatis vitae, justo. Nullam dictum felis eu pede mollis pretium. Integer tincidunt. Cras dapibus. Vivamus elementum semper nisi. Aenean vulputate eleifend tellus. Aenean leo ligula, porttitor eu, consequat vitae, eleifend ac, enim. Aliquam lorem ante, dapibus in, viverra quis, feugiat a, tellus. Phasellus viverra nulla ut metus varius laoreet. Quisque rutrum. Aenean imperdiet. Etiam ultricies nisi vel augue. Curabitur ullamcorper ultricies nisi. Nam eget dui. Etiam rhoncus. Maecenas tempus, tellus eget condimentum rhoncus, sem quam semper libero, sit amet adipiscing sem neque sed ipsum. Nam quam nunc, blandit  \n";
    return res;
}

class FCGIParamsView{
public:
    FCGIParamsView(char** params)
    : params_{params}
{}

bool hasParam(const std::string& name) const
{
    auto it = params_;
    while (it != nullptr) {
        auto paramStr = std::string_view{*it};
        if (paramStr.size() >= name.size() && paramStr.substr(0, name.size()) == name)
            return true;
        it++;
    }
    return false;
}

std::string_view param(const std::string& name) const
{
    auto it = params_;
    while (it != nullptr) {
        auto paramStr = std::string_view{*it};
        if (paramStr.size() >= name.size() && paramStr.substr(0, name.size()) == name) {
            auto paramVal = paramStr.substr(name.size(), paramStr.size() - name.size());
            if (!paramVal.empty() && paramVal.front() == '=') {
                paramVal.remove_prefix(1);
                return paramVal;
            }
            else
                throw std::runtime_error{"Parameter '" + name + "' has invalid format"};
        }
        it++;
    }
    throw std::out_of_range{"Parameter '" + name + "' not found"};
}

private:
    char** params_;
};

class FCGIOutStream : public std::ostream
{
public:
    FCGIOutStream(FCGX_Stream* stream)
        : buf_(stream)
    {
        rdbuf(&buf_);
    }
private:
    fcgi_streambuf buf_;
};

struct Cfg : public cmdlime::GNUConfig {
    CMDLIME_PARAM(responseSize, int) << "response size in kb"
                                     << [](int value) {
                                         if (value < 1)
                                             throw cmdlime::ValidationError{"response size must be greater than 0"};
                                     };
};

int main(int argc, char** argv)
{
    auto cfg = Cfg{};
    auto configReader = cmdlime::ConfigReader{cfg, "libfcgi_benchmark"};
    if (!configReader.readCommandLine(argc, argv))
        return configReader.exitCode();

    const auto unixSocket = "/tmp/fcgi.sock";
    FCGX_Init();
    umask(0);
    const auto socket = FCGX_OpenSocket(unixSocket, 1024);
    chmod(unixSocket, 0777);
    FCGX_Request request;
    if (FCGX_InitRequest(&request, socket, FCGI_FAIL_ACCEPT_ON_INTR) != 0)
        return 1;

    static auto payload = generateText(100);
    const auto response = std::string{"HTTP/1.1 200 OK\r\n "
                     "Content-Type: text/html\r\n"
                     "\r\n"
                     + payload};
    while(FCGX_Accept_r(&request) == 0){
        if (request.envp == nullptr){
            std::cout << "Can't process data without FCGI params";
            continue;
        }

        const auto contentLength = [&]
        {
            auto params = FCGIParamsView{request.envp};
            if (!params.hasParam("CONTENT_LENGTH"))
                return 0;
            auto lengthStr = params.param("CONTENT_LENGTH");
            if (lengthStr.empty())
                return 0;
            return std::stoi(std::string{lengthStr});
        }();

        auto fcgiData = std::string{};
        if (contentLength > 0){
            fcgiData.resize(static_cast<std::size_t>(contentLength));
            FCGX_GetStr(&fcgiData[0], contentLength, request.in);
        }

        FCGIOutStream outStream{request.out};
        outStream.write(response.data(), static_cast<std::streamsize>(response.size()));
        FCGX_Finish_r(&request);
    }
    return 0;
}
