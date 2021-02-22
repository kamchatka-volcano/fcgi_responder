#pragma once

namespace fcgi{

class Request;
class MsgStdIn;
class MsgParams;

class RequestEditor{
public:
    RequestEditor(Request& request);
    void addStdInMsg(const fcgi::MsgStdIn& msg);
    void addParamsMsg(const fcgi::MsgParams& msg);

private:
    Request& request_;
};

}
