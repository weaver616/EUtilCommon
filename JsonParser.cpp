#include <stdafx.h>
#include <string>
#include <map>
#include <vector>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <JsonParser.h>


#pragma region json解析
std::string value_to_string(const rapidjson::Value &value)
{
    if (value.IsString())
        return value.GetString();
    else if (value.IsInt())
        return std::to_string(value.GetInt());
    else if (value.IsUint())
        return std::to_string(value.GetUint());
    else if (value.IsInt64())
        return std::to_string(value.GetInt64());
    else if (value.IsUint64())
        return std::to_string(value.GetUint64());
    else if (value.IsDouble())
        return std::to_string(value.GetDouble());
    else if (value.IsBool())
        return value.GetBool() ? "true" : "false";
    else if (value.IsNull())
        return "null";
    else
    {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        value.Accept(writer);
        return buffer.GetString();
    }
}

LevelParser::LevelParser(int level_) : level(level_) {}

jsblocks LevelParser::parse(const std::string json_str)
{
    jsblocks blocks;
    rapidjson::Document doc;
    doc.Parse(json_str.c_str());

    if (doc.HasParseError())
    {
        std::cerr << "Parse error!\n";
        return {};
    }

    if (doc.IsObject())
    {
        jsblocks result = process_object(doc, "root", 0);
        blocks.insert(blocks.end(), result.begin(), result.end());
    }
    else if (doc.IsArray())
    {
        jsblocks result = process_array(doc, "root_array", 0);
        blocks.insert(blocks.end(), result.begin(), result.end());
    }

    return blocks;
}
//  实现 printParsed
void LevelParser::printParsed(const jsblocks &data) const
{
    for (const auto &table : data)
    {
        std::cout << "In LevelParser  Class === Table: " << table.tname << " (" << table.tdataset.size() << " rows) ===\n";
        for (const auto &row : table.tdataset)
        {
            for (const auto &col : row)
            {
                std::cout << "  " << col.first << ": " << col.second << "\n";
            }
        }
    }
}

jsblocks LevelParser::process_object(const rapidjson::Value &obj, const std::string &table_name, int current_level)
{
    jsblocks result;

    if (current_level >= level)
    {
        jstable table{table_name, {}};
        std::map<std::string, std::string> row;
        for (auto it = obj.MemberBegin(); it != obj.MemberEnd(); ++it)
        {
            if (it->name.IsString())
            {
                row[it->name.GetString()] = value_to_string(it->value);
            }
        }
        table.tdataset.push_back(row);
        result.push_back(std::move(table));
    }
    else
    {
        for (auto it = obj.MemberBegin(); it != obj.MemberEnd(); ++it)
        {
            if (!it->name.IsString())
                continue;
            const std::string key = it->name.GetString();
            const auto &val = it->value;

            if (val.IsObject())
            {
                jsblocks child = process_object(val, key, current_level + 1);
                result.insert(result.end(), child.begin(), child.end());
            }
            else if (val.IsArray())
            {
                jsblocks child = process_array(val, key, current_level + 1);
                result.insert(result.end(), child.begin(), child.end());
            }
        }
    }

    return result;
}

jsblocks LevelParser::process_array(const rapidjson::Value &arr, const std::string &table_name, int current_level)
{
    jsblocks result;

    if (current_level >= level)
    {
        jstable table{table_name, {}};
        for (const auto &val : arr.GetArray())
        {
            std::map<std::string, std::string> row;
            if (val.IsObject())
            {
                for (auto it = val.MemberBegin(); it != val.MemberEnd(); ++it)
                {
                    if (it->name.IsString())
                    {
                        row[it->name.GetString()] = value_to_string(it->value);
                    }
                }
            }
            else
            {
                row["value"] = value_to_string(val);
            }
            table.tdataset.push_back(row);
        }
        result.push_back(std::move(table));
    }
    else
    {
        for (const auto &item : arr.GetArray())
        {
            if (item.IsObject())
            {
                jsblocks child = process_object(item, table_name, current_level + 1);
                result.insert(result.end(), child.begin(), child.end());
            }
            else if (item.IsArray())
            {
                jsblocks child = process_array(item, table_name, current_level + 1);
                result.insert(result.end(), child.begin(), child.end());
            }
        }
    }

    return result;
}

ParserRegistry &ParserRegistry::instance()
{
    static ParserRegistry registry;
    return registry;
}

void ParserRegistry::registerParser(const std::string &name, CreatorFunc creator)
{
    creators[name] = creator;
}
std::unique_ptr<IJsonParser> ParserRegistry::create(const std::string &name, int level)
{
    auto it = creators.find(name);
    if (it != creators.end())
    {
        return it->second(level);
    }
    throw std::runtime_error("Unknown parser: " + name);
}

// LevelParserRegister::LevelParserRegister()
// {
//     ParserRegistry::instance().registerParser(
//         "LevelParser",
//         [](int level) -> std::unique_ptr<IJsonParser>
//         {
//             return std::make_unique<LevelParser>(level);
//         });
// }

LevelParserRegister::LevelParserRegister()
{
    ParserRegistry::instance().registerParser(
        "LevelParser",
        [](int level) -> std::unique_ptr<IJsonParser>
        {
            return std::unique_ptr<IJsonParser>(new LevelParser(level));
        });
}

LevelParserRegister _levelParserRegister;

#pragma endregion  json解析


// int mian()
// {
//     // 创建一个测试JSON字符串
//     std::string json_text = R"(
//     {
//         "name": "John",
//         "age": 30,
//         "skills": ["C++", "Python", "JSON"],
//         "education": {
//             "degree": "Master",
//             "year": 2015
//         },
//         "projects": [
//             {"title": "Project A", "year": 2020},
//             {"title": "Project B", "year": 2021}
//         ]
//     }
//     )";

//     // 创建 LevelParser
//     std::unique_ptr<IJsonParser> parser = ParserRegistry::instance().create("LevelParser", 1);

//     // 解析
//     jsblocks tables = parser->parse(json_text);

//     // 输出解析结果
//     parser->printParsed(tables);

//     return 0;
// }
