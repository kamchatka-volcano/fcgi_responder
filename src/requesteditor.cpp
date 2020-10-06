#include "requesteditor.h"
#include "request.h"
#include "streamdatamessage.h"
#include "msgparams.h"

using namespace fcgi;

RequestEditor::RequestEditor(Request& request)
    : request_(request)
{
}

void RequestEditor::addStdInMsg(const fcgi::MsgStdIn& msg)
{
    request_.stdIn_ += msg.data();
}

void RequestEditor::addParamsMsg(const fcgi::MsgParams& msg)
{
    for(const auto& paramName : msg.paramList())
        request_.params_[paramName] = msg.paramValue(paramName);
}

void RequestEditor::setKeepConnection(bool state)
{
    request_.keepConnection_ = state;
}
