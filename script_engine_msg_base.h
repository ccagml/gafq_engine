#ifndef SCRIPT_ENGINE_MSG_BASE_H
#define SCRIPT_ENGINE_MSG_BASE_H

class ScriptEngineMsgBase
{

public:
    ScriptEngineMsgBase() { std::cout << "create enginemsg base" << std::endl; };
    ~ScriptEngineMsgBase() { std::cout << "delete enginemsg base" << std::endl; };
    std::string action;
    std::string data;

private:
};

#endif