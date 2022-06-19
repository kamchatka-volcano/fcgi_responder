#include <fcgi_responder/responder.h>
#include <cmdlime/gnuconfig.h>
#include <filesystem>
#include <iterator>
#include <fstream>
#include <iostream>

std::string peek(std::istream& stream, int size = 1)
{
    auto result = std::string{};
    auto ch = char{};
    auto pos = stream.tellg();
    for (auto i = 0; i < size; ++i){
        if (!stream.get(ch)){
            stream.clear();
            result.clear();
            break;
        }
        result.push_back(ch);
    }
    stream.seekg(pos);
    return result;
}

void skip(std::istream& stream, int size)
{
    auto ch = char{};
    for (auto i = 0; i < size; ++i)
        stream.get(ch);
}

class Responder : public fcgi::Responder{
public:
    Responder()
    {
        setErrorInfoHandler([](const std::string& error){
            std::cerr << error << std::endl;
        });
    }

    void loadInput(const std::filesystem::path& path)
    {
        auto stream = std::ifstream{path, std::ios::binary};
        auto buffer = std::string{};
        while (stream.good()){
            if (peek(stream, 8) == "NEWLINE!") {
                const auto bufferSize = buffer.size();
                receiveData(buffer.data(), bufferSize);
                buffer.clear();
                skip(stream, 8);
                continue;
            }
            auto ch = char{};
            if (!stream.get(ch))
                break;
            buffer.push_back(ch);
        }
        const auto bufferSize = buffer.size();
        receiveData(buffer.data(), bufferSize);
    }


private:
    void sendData(const std::string& data) override
    {
        std::cout << data;
    }

    void disconnect() override
    {}
    void processRequest(fcgi::Request&&, fcgi::Response&&) override
    {}
};

struct CmdlineCfg : public cmdlime::GNUConfig{
    CMDLIME_ARG(inputFile, std::filesystem::path) << "input file"
    << [](const auto& path){
        if (!std::filesystem::exists(path))
            throw cmdlime::ValidationError{"file does not exist"};
       };
};

int main(int argc, char** argv)
{
    auto cmdl = CmdlineCfg{};
    auto reader = cmdlime::ConfigReader{cmdl, "fcgi_responder_fuzzer"};
    if (!reader.readCommandLine(argc, argv))
        return reader.exitCode();

    auto responder = Responder();
    responder.loadInput(cmdl.inputFile);
    return 0;
}