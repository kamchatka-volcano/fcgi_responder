#include "streammaker.h"
#include "streamdatamessage.h"

namespace fcgi{

StreamMaker::StreamMaker(std::size_t maxDataMessageSize)
    : maxDataMessageSize_(maxDataMessageSize)
{
}

}
