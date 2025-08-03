#pragma once
#include <stdafx.h>
#include <JsonParser.h>
// EIClass 构建器
class EIClassBuilder {
public:
    EIClassBuilder(const std::string& parserName, int level = 0);
    EIClass fromJson(const std::string& json);

private: 
    std::string sanitizeTableName(const std::string &name) const;
    std::unique_ptr<IJsonParser> parser;
};

#pragma region 系统命令

// class Process
// {
// public:
//     explicit Process(const std::string &cmd);
//     ~Process();    
//     std::string readAll(unsigned int timeout_ms = 0);

// private:
//     FILE *pipe_;
//     std::string cmd_;
// private:
//      std::string run() ;
// };

class Process
{
    public:
    static std::string execute(const std::string& cmd);
};

#pragma endregion 系统命令