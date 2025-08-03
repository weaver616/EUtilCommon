#pragma once
#include <stdafx.h>
#include <map>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <json/json.h> 

struct jstable
{
    std::string tname;
    std::vector<std::map<std::string, std::string>> tdataset;
};

using jsblocks = std::vector<jstable>;

class IJsonParser
{
public:
    virtual ~IJsonParser() {}
    virtual jsblocks parse(const std::string &json) = 0;
    virtual void printParsed(const jsblocks &data) const = 0;
};

// LevelParser
class LevelParser : public IJsonParser
{
public:
    explicit LevelParser(int level_);
    jsblocks parse(const std::string &json_str) override;
    void printParsed(const jsblocks &data) const override;

private:
    int level;
    // jsblocks process_object(const rapidjson::Value &obj, const std::string &table_name, int current_level);
    jsblocks process_object(const Json::Value &obj, const std::string &table_name, int current_level);
    // jsblocks process_array(const rapidjson::Value &arr, const std::string &table_name, int current_level);
    jsblocks process_array(const Json::Value&, const string&, int);
    
};


// ParserRegistry
class ParserRegistry
{
public:
    using CreatorFunc = std::function<std::unique_ptr<IJsonParser>(int)>;
    static ParserRegistry &instance();
    void registerParser(const std::string &name, CreatorFunc creator);
    std::unique_ptr<IJsonParser> create(const std::string &name, int level);

private:
    std::map<std::string, CreatorFunc> creators;
};

struct LevelParserRegister
{
    LevelParserRegister();
};