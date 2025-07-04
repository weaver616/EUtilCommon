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

class Process
{
public:
    explicit Process(const std::string &cmd);
    ~Process();
    std::string readAll();

private:
    FILE *pipe_;
};

#pragma endregion 系统命令